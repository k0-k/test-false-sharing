#define _GNU_SOURCE
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <x86intrin.h>
#include "thread.h"

typedef volatile uint64_t (block_t)[8];
static block_t blocks[8] __attribute__((__aligned__(64)));

struct writers_cond {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	int inv_ready;
};

static struct writers_cond writers_cond = {
	.cond = PTHREAD_COND_INITIALIZER,
	.mutex = PTHREAD_MUTEX_INITIALIZER,
};

#define prepare_thread(args_)						\
	struct args *args = args_;					\
	const int cpu = args->cpu;					\
	const int x = args->index.x, y = args->index.y;			\
	const int nb_loops = args->nb_loops;				\
	free(args_);							\
									\
	struct results *results = calloc(1, sizeof *results);		\
	if (!results)							\
		return NULL;						\
									\
	pthread_t tid = pthread_self();					\
	cpu_set_t cpuset;						\
	CPU_ZERO(&cpuset);						\
	CPU_SET(cpu, &cpuset);						\
	if (pthread_setaffinity_np(tid, sizeof cpuset, &cpuset)) {	\
		fprintf(stderr, "error occurred in %s\n", __func__);	\
	}

static int
wake_reader(void)
{
	_Bool do_signal;
	pthread_mutex_lock(&(writers_cond.mutex));
	writers_cond.inv_ready--;
	do_signal = (writers_cond.inv_ready <= 0);
	pthread_mutex_unlock(&(writers_cond.mutex));

	if (!writers_cond.inv_ready)
		pthread_cond_signal(&(writers_cond.cond));

	return 0;
}

static int
sleep_reader(void)
{
	pthread_mutex_lock(&(writers_cond.mutex));
	while (writers_cond.inv_ready) {
		pthread_cond_wait(&(writers_cond.cond),
				&(writers_cond.mutex));
	}
	pthread_mutex_unlock(&(writers_cond.mutex));

	return 0;
}

void *
writer_blind_write(void *args_)
{
	prepare_thread(args_);
	wake_reader();

	uint64_t i;
	unsigned long long t1, t2;

	t1 = __rdtsc();
	_mm_mfence();
	for (i = 0; i < nb_loops; i++) {
		blocks[x][y] = i;
	}
	_mm_mfence();
	t2 = __rdtsc();

	results->nb_loops = i;
	results->delta = t2 - t1;
	return results;
}

void *
writer_read_modify_write(void *args_)
{
	prepare_thread(args_);
	wake_reader();

	uint64_t i;
	unsigned long long t1, t2;

	t1 = __rdtsc();
	_mm_mfence();
	for (i = 0; i < nb_loops; i++) {
		blocks[x][y]++;
	}
	_mm_mfence();
	t2 = __rdtsc();

	results->nb_loops = i;
	results->delta = t2 - t1;
	return results;
}

void *
reader(void *args_)
{
	prepare_thread(args_);
	sleep_reader();

	int i;
	uint64_t v;
	unsigned long long t1, t2;

	t1 = __rdtsc();
	_mm_mfence();
	for (i = 0; i < nb_loops; i++) {
		v = blocks[x][y];
	}
	_mm_mfence();
	t2 = __rdtsc();

	results->nb_loops = i;
	results->delta = t2 - t1;
	return results;
}

int
initialize_threads_params(const struct threads_params *params)
{
	writers_cond.inv_ready = params->nb_writers;
	return 0;
}
