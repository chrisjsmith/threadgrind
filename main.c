#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MAX_THREADS 128

typedef struct {
	int core;
	int result;
} grindt;

pthread_t threads[MAX_THREADS];
grindt tmap[MAX_THREADS]; /* thread data */
time_t objective;

/* courtesy Homer. I don't think he'll mind */
char *tohash = "Meanwhile the people were gathered in assembly, for there "
	"was a quarrel, and two men were wrangling about the blood-money for "
	"a man who had been killed, the one saying before the people that he "
	"had paid damages in full, and the other that he had not been paid. "
	"Each was trying to make his own case good, and the people took "
	"sides, each man backing the side that he had taken; but the heralds "
	"kept them back, and the elders sate on their seats of stone in a "
	"solemn circle, holding the staves which the heralds had put into "
	"their hands. Then they rose and each in his turn gave judgement, "
	"and there were two talents laid down, to be given to him whose "
	"judgement should be deemed the fairest.";

/* DJB hash */
int djb2(char *str) {
	unsigned long i = 5381;
	int c;

	while (c = *str++)
		i = ((i << 5) + i) + c;
	return i;
}

/* worker thread */
void *cpu_worker(void *p) {
	time_t now;
	int i, j;
	cpu_set_t cpuset;
	pthread_t t;
	grindt *gt;

	gt = (grindt *) p;

	/* set affinity to provided core */
	CPU_ZERO(&cpuset);
	CPU_SET(gt->core, &cpuset);
	t = pthread_self();
	pthread_setaffinity_np(t, sizeof(cpu_set_t), &cpuset);

	/* do work until time is past objective */
	printf("Worker started on core %d\n", gt->core);
	for (i=0; ; i++) {
		for (j=0; j < 1000; j++) 
			djb2(tohash);
		now = time(NULL);
		if (now > objective)
			break;	
	}

	/* save iteration count as result */
	gt->result = i;
	return NULL;
}

/* comparer for results */
int grindt_compare_result(const void* a, const void* b) {
	grindt *grindt_a = (grindt *)a;
	grindt *grindt_b = (grindt *)b;

	if (grindt_a->result < grindt_b->result)
		return -1;
	if (grindt_a->result > grindt_b->result)
		return 1;
	return 0;
}

/* entry point */
int main(int argc, char *argv[]) {
	int i, nprocs, nthreads;

	/* work out logical processor count */
	nprocs = sysconf(_SC_NPROCESSORS_ONLN);
	printf("Detected %d logical processors online\n", nprocs);

	/* override logical processor count with parameter if provided */
	if (argc == 2) {
		if (sscanf(argv[1], "%d", &nthreads) == EOF ||
			nthreads > MAX_THREADS ||
			nthreads < 1) {
			fprintf(stderr, "invalid thread count");
			return 1;
		}
	} else
		nthreads = nprocs;
	printf("Using %d threads\n", nthreads);

	/* work out 10 seconds in the future */
	objective = time(NULL) + 10;

	/* create threads */
	for (i = 0; i < nthreads; i++) {
		tmap[i].core = i % nprocs;
		pthread_create(&threads[i], NULL, cpu_worker, &tmap[i]);
	}

	/* wait for all threads to exit */
       	for (i = 0; i < nthreads; i++) 
		pthread_join(threads[i], NULL);	

	/* sort results */
	qsort(tmap, nthreads, sizeof(grindt), grindt_compare_result);

	/* print results */
	for (i = 0; i < nthreads; i++)
		printf("Iteration blocks on core %d: %d\n", tmap[i].core, tmap[i].result);

	return 0;
}
