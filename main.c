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

	pthread_t *r_th[cli_options->nb_readers];
	pthread_t *w_th[cli_options->nb_writers];

	const struct threads_params params = {
		.nb_readers = cli_options->nb_readers,
		.nb_writers = cli_options->nb_writers,
	};
	initialize_threads_params(&params);

	int cpuid = 0;
	thread_entrypoint_t writer = cli_options->writer;
	for (int i = 0; i < cli_options->nb_writers; i++, cpuid++) {
		pthread_t *th = create_thread(writer, cpuid, cli_options->block_index.writer, i, cli_options->nb_loops);
		if (!th)
			return -1;
		w_th[i] = th;
	}

	for (int i = 0; i < cli_options->nb_readers; i++, cpuid++) {
		pthread_t *th = create_thread(reader, cpuid, cli_options->block_index.reader, 7, cli_options->nb_loops);
		if (!th)
			return -1;
		r_th[i] = th;
	}

	while (try_wake_threads(cli_options->nb_writers + cli_options->nb_readers)) {
		;
	}

	for (int i = 0; i < cli_options->nb_writers; i++) {
		struct results *results = NULL;
		if (pthread_join(*w_th[i], (void **)&results) < 0) {
			return -1;
		}

		printf("writer(%p): delta=%llu cycles=%Lf\n",
				(void *)w_th[i],
				results->delta,
				(long double) results->delta / results->nb_loops
		);
		free(results);
		free(w_th[i]);
	}

	for (int i = 0; i < cli_options->nb_readers; i++) {
		struct results *results = NULL;
		if (pthread_join(*r_th[i], (void **)&results) < 0) {
			return -1;
		}

		printf("reader(%p): delta=%llu cycles=%Lf\n",
				(void *)r_th[i],
				results->delta,
				(long double) results->delta / results->nb_loops
		);
		free(results);
		free(r_th[i]);
	}

	return 0;
}
