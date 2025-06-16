#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define MAX_THREADS 128

pthread_t threads[MAX_THREADS];
int cmap[MAX_THREADS]; /* lazy hack to avoid malloc */
time_t objective;

/* courtesy Homer. I don't think he'll mind */
char *tohash = "Meanwhile the people were gathered in assembly, for there was a quarrel, and two men were wrangling about the blood-money for a man who had been killed, the one saying before the people that he had paid damages in full, and the other that he had not been paid. Each was trying to make his own case good, and the people took sides, each man backing the side that he had taken; but the heralds kept them back, and the elders sate on their seats of stone in a solemn circle, holding the staves which the heralds had put into their hands. Then they rose and each in his turn gave judgement, and there were two talents laid down, to be given to him whose judgement should be deemed the fairest.";

int djb2(char *str) {
	unsigned long i = 5381;
	int c;
	while (c = *str++)
		i = ((i << 5) + i) + c;
	return i;
		
}

void *cpu_worker(void *p) {
	time_t now;
	int i, j, *core;
	cpu_set_t cpuset;
	pthread_t t;

	core = (int*)p;

	CPU_ZERO(&cpuset);
	CPU_SET(*core, &cpuset);
	t = pthread_self();
	pthread_setaffinity_np(t, sizeof(cpu_set_t), &cpuset);
	printf("Worker started on core %d\n", *core);
	for (i=0; ; i++) {
		for (j=0; j < 1000; j++) 
			djb2(tohash);
		now = time(NULL);
		if (now > objective)
			break;	
	}
	printf("Iteration blocks on core %d: %d\n", *core, i);
	return NULL;
}

int main(int argc, char *argv[]) {
	int i, nthreads;
	nthreads = sysconf(_SC_NPROCESSORS_ONLN);
	printf("Detected %d logical processors online\n", nthreads);
	objective = time(NULL) + 10;
	for (i = 0; i < nthreads; i++) {
		cmap[i] = i;
		pthread_create(&threads[i], NULL, cpu_worker, &cmap[i]); 
	}
       	for (i = 0; i < nthreads; i++) 
		pthread_join(threads[i], NULL);	
	return 0;
}
