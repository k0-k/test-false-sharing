#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "cli_options.h"
#include "thread.h"

int
main(int argc, char *argv[])
{
	const struct cli_options *cli_options = parse(argc, argv);
	if (!cli_options)
		return -1;

	const int nb_threads0 = cli_options->threads_group[0].nb;
	const int nb_threads1 = cli_options->threads_group[1].nb;
	pthread_t *group0[nb_threads0];
	pthread_t *group1[nb_threads1];
	thread_entrypoint_t entrypoint0 = cli_options->threads_group[0].entrypoint;
	thread_entrypoint_t entrypoint1 = cli_options->threads_group[1].entrypoint;

	int cpuid = 0;

	for (int i = 0; i < nb_threads0; i++, cpuid++) {
		const int block_index = cli_options->threads_group[0].block_index;
		pthread_t *th = create_thread(entrypoint0, cpuid, block_index, 7, cli_options->nb_loops);
		if (!th)
			return -1;
		group0[i] = th;
	}

	for (int i = 0; i < nb_threads1; i++, cpuid++) {
		const int block_index = cli_options->threads_group[1].block_index;
		pthread_t *th = create_thread(entrypoint1, cpuid, block_index, i, cli_options->nb_loops);
		if (!th)
			return -1;
		group1[i] = th;
	}

	while (try_wake_threads(nb_threads0 + nb_threads1)) {
		;
	}

	for (int i = 0; i < nb_threads0; i++) {
		struct results *results = join_thread(group0[i]);
		if (!results) {
			return -1;
		}

		printf("%s(%p): delta=%llu cycles=%Lf\n",
				results->thread_kind,
				(void *)group0[i],
				results->delta,
				(long double) results->delta / results->nb_loops
		);
		free(results);
	}

	for (int i = 0; i < nb_threads1; i++) {
		struct results *results = join_thread(group1[i]);
		if (!results) {
			return -1;
		}

		printf("%s(%p): delta=%llu cycles=%Lf\n",
				results->thread_kind,
				(void *)group1[i],
				results->delta,
				(long double) results->delta / results->nb_loops
		);
		free(results);
	}

	return 0;
}
