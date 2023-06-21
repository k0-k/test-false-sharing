#pragma once
#define _GNU_SOURCE

#include <pthread.h>

struct threads_params {
	int nb_readers, nb_writers;
};

int initialize_threads_params(const struct threads_params *);

struct args {
	int cpuid;
	struct {
		int x, y;
	} index;
	int nb_loops;
};

struct results {
	int nb_loops;
	unsigned long long delta;
};

int try_wake_threads(const int expected_nb_threads);

void *writer_blind_write(void *);
void *writer_read_modify_write(void *);
void *reader(void *);

typedef void *(*thread_entrypoint_t)(void *args);
pthread_t * create_thread(thread_entrypoint_t entrypoint, int cpuid, int x, int y, int nb_loops);
struct results * join_thread(pthread_t *th);
