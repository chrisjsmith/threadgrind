# threadgrind

Hammer one thread per core on a Linux node to assess relative CPU performance. Uses naive djb2 hash iterations.

Allows you to infer E-cores vs P-cores and cloud provider ripping you off for hyperthreads instead of real cores.

## Build

1. Check out.
2. Make sure you have GCC, GNU Make installed.
3. Run `make`
4. Out pops `threadgrind`

## Usage

* `./threadgrind` - detect core count and run a thread per core.
* `./threadgrind n` - override number of threads and balance them fairly across cores.
