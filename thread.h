#pragma once
#define _GNU_SOURCE

#include <pthread.h>

struct args {
	int cpuid;
	struct {
		int x, y;
	} index;
	int nb_loops;
};

struct results {
	const char *thread_kind;
	int nb_loops;
	unsigned long long delta;
};

int try_wake_threads(const int expected_nb_threads);

void *thread_entrypoint_writer_blind_write(void *);
void *thread_entrypoint_writer_read_modify_write(void *);
void *thread_entrypoint_reader(void *);

typedef void *(*thread_entrypoint_t)(void *args);
pthread_t * create_thread(thread_entrypoint_t entrypoint, int cpuid, int x, int y, int nb_loops);
struct results * join_thread(pthread_t *th);
