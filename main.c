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

	const int nb_readers = cli_options->threads_config.r_w.readers.nb;
	const int nb_writers = cli_options->threads_config.r_w.writers.nb;

	pthread_t *r_th[nb_readers];
	pthread_t *w_th[nb_writers];

	int cpuid = 0;
	thread_entrypoint_t writer = cli_options->threads_config.r_w.writers.entrypoint;
	for (int i = 0; i < nb_writers; i++, cpuid++) {
		const int block_index = cli_options->threads_config.r_w.writers.block_index;
		pthread_t *th = create_thread(writer, cpuid, block_index, i, cli_options->nb_loops);
		if (!th)
			return -1;
		w_th[i] = th;
	}

	thread_entrypoint_t reader = cli_options->threads_config.r_w.readers.entrypoint;
	for (int i = 0; i < nb_readers; i++, cpuid++) {
		const int block_index = cli_options->threads_config.r_w.readers.block_index;
		pthread_t *th = create_thread(reader, cpuid, block_index, 7, cli_options->nb_loops);
		if (!th)
			return -1;
		r_th[i] = th;
	}

	while (try_wake_threads(nb_writers + nb_readers)) {
		;
	}

	for (int i = 0; i < nb_writers; i++) {
		struct results *results = join_thread(w_th[i]);
		if (!results) {
			return -1;
		}

		printf("%s(%p): delta=%llu cycles=%Lf\n",
				results->thread_kind,
				(void *)w_th[i],
				results->delta,
				(long double) results->delta / results->nb_loops
		);
		free(results);
	}

	for (int i = 0; i < nb_readers; i++) {
		struct results *results = join_thread(r_th[i]);
		if (!results) {
			return -1;
		}

		printf("%s(%p): delta=%llu cycles=%Lf\n",
				results->thread_kind,
				(void *)r_th[i],
				results->delta,
				(long double) results->delta / results->nb_loops
		);
		free(results);
	}

	return 0;
}
