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

	pthread_t r_th[cli_options->nb_readers];
	pthread_t w_th[cli_options->nb_writers];
	const struct threads_params params = {
		.nb_readers = cli_options->nb_readers,
		.nb_writers = cli_options->nb_writers,
	};
	initialize_threads_params(&params);

	int cpu = 0;
	void *(*writer)(void *args) = cli_options->writer;
	for (int i = 0; i < cli_options->nb_writers; i++) {
		struct args *args = calloc(1, sizeof *args);
		if (!args)
			return -1;

		args->cpu = cpu++;
		args->index.x = cli_options->block_index.writer;
		args->index.y = i;
		args->nb_loops = cli_options->nb_loops;
		if (pthread_create(&w_th[i], NULL, writer, args) < 0) {
			return -1;
		}
	}

	for (int i = 0; i < cli_options->nb_readers; i++) {
		struct args *args = calloc(1, sizeof *args);
		if (!args)
			return -1;

		args->cpu = cpu++;
		args->index.x = cli_options->block_index.reader;
		args->index.y = 7;
		args->nb_loops = cli_options->nb_loops;
		if (pthread_create(&r_th[i], NULL, reader, args) < 0) {
			return -1;
		}
	}

	for (int i = 0; i < cli_options->nb_writers; i++) {
		struct results *results = NULL;
		if (pthread_join(w_th[i], (void **)&results) < 0) {
			return -1;
		}

		printf("writer(%p): delta=%llu cycles=%Lf\n",
				(void *)w_th[i],
				results->delta,
				(long double) results->delta / results->nb_loops
		);
		free(results);
	}

	for (int i = 0; i < cli_options->nb_readers; i++) {
		struct results *results = NULL;
		if (pthread_join(r_th[i], (void **)&results) < 0) {
			return -1;
		}

		printf("reader(%p): delta=%llu cycles=%Lf\n",
				(void *)r_th[i],
				results->delta,
				(long double) results->delta / results->nb_loops
		);
		free(results);
	}

	return 0;
}
