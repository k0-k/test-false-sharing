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

struct threads_cond {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	int nb_slept;
	_Bool awake;
};

static struct threads_cond threads_cond = {
	.cond = PTHREAD_COND_INITIALIZER,
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.awake = false,
};

#define PREPARE_THREAD(args_)						\
	struct args *args = args_;					\
	const int cpuid = args->cpuid;					\
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
	CPU_SET(cpuid, &cpuset);					\
	if (pthread_setaffinity_np(tid, sizeof cpuset, &cpuset)) {	\
		fprintf(stderr, "error occurred in %s\n", __func__);	\
	}

int
try_wake_threads(const int expected_nb_threads)
{
	pthread_mutex_lock(&(threads_cond.mutex));

	_Bool do_signal = (threads_cond.nb_slept == expected_nb_threads);
	if (do_signal) {
		threads_cond.awake = true;
		pthread_cond_broadcast(&(threads_cond.cond));
	}

	pthread_mutex_unlock(&(threads_cond.mutex));

	return (int)!do_signal;
}

static int
sleep_thread(void)
{
	pthread_mutex_lock(&(threads_cond.mutex));
	threads_cond.nb_slept++;
	while (!threads_cond.awake)
		pthread_cond_wait(&(threads_cond.cond),
				&(threads_cond.mutex));
	pthread_mutex_unlock(&(threads_cond.mutex));

	return 0;
}

void *
writer_blind_write(void *args_)
{
	PREPARE_THREAD(args_);

	uint64_t i;
	unsigned long long t1, t2;

	sleep_thread();

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
	PREPARE_THREAD(args_);

	uint64_t i;
	unsigned long long t1, t2;

	sleep_thread();

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
	PREPARE_THREAD(args_);

	int i;
	uint64_t v;
	unsigned long long t1, t2;

	sleep_thread();

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

typedef void *(*entrypoint_t)(void *args);

pthread_t *
create_thread(thread_entrypoint_t entrypoint, int cpuid, int x, int y, int nb_loops)
{
	struct args *args = calloc(1, sizeof *args);
	pthread_t *th = calloc(1, sizeof *th);
	if (!args || !th) {
		goto error;
	}

	args->cpuid = cpuid;
	args->index.x = x; 
	args->index.y = y;
	args->nb_loops = nb_loops;
	if (pthread_create(th, NULL, entrypoint, args) < 0) {
		goto error;
	}

	return th;
error:
	free(th);
	free(args);
	return NULL;
}

struct results *
join_thread(pthread_t *th)
{
	struct results *results = NULL;
	if (pthread_join(*th, (void **)&results) < 0) {
		return NULL;
	}
	free(th);
	return results;
}

int
initialize_threads_params(const struct threads_params *params)
{
	return 0;
}
