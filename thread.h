#pragma once
#define _GNU_SOURCE

struct threads_params {
	int nb_readers, nb_writers;
};

int initialize_threads_params(const struct threads_params *);

struct args {
	int cpu;
	struct {
		int x, y;
	} index;
	int nb_loops;
};

struct results {
	int nb_loops;
	unsigned long long delta;
};

void *writer(void *);
void *reader(void *);
