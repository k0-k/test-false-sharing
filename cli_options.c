#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include "cli_options.h"
#include "thread.h"

#define NB_LOOPS	(2000000)
#define NB_READERS	(2)
#define NB_WRITERS	(2)
#define MEM_BLK_READER	(0)
#define MEM_BLK_WRITER	(0)

static struct cli_options cli_options = {
	.writer = writer_blind_write,
	.nb_loops = NB_LOOPS,
	.nb_readers = NB_READERS,
	.nb_writers = NB_WRITERS,
	.block_index = {
		.reader = MEM_BLK_READER,
		.writer = MEM_BLK_WRITER,
	},
};

static int
get_nb_cpu(void)
{
	int nb_cpu = sysconf(_SC_NPROCESSORS_ONLN);
	if (nb_cpu < 0)
		perror("sysconf");

	return nb_cpu;
}

const struct cli_options *
parse(int argc, char *argv[])
{
	const int nb_cpu = get_nb_cpu();
	struct option options[] = {
		{"nb_loops", required_argument, NULL, 'l'},
		{"nb_readers", required_argument, NULL, 'r'},
		{"write_pattern", required_argument, NULL, 'p'},
		{"nb_writers", required_argument, NULL, 'w'},
		{"mb_readers", required_argument, NULL, 'R'},
		{"mb_writers", required_argument, NULL, 'W'},
		{NULL, 0, NULL, 0},
	};
	const struct cli_options *result = NULL;

	for (_Bool is_done = false; is_done != true;) {
		int c = getopt_long(argc, argv, "r:w:l:R:W:p:",
				options, NULL);
		int v;

		switch (c) {
		case 'l':
			sscanf(optarg, "%d", &v);
			if (v < 0)
				goto end;

			cli_options.nb_loops = v;
			break;
		case 'r':
			sscanf(optarg, "%d", &v);
			if (v < 0)
				goto end;

			cli_options.nb_readers = v;
			break;
		case 'w':
			sscanf(optarg, "%d", &v);
			if (v < 0)
				goto end;

			cli_options.nb_writers = v;
			break;
		case 'R':
			sscanf(optarg, "%d", &v);
			if (v < 0 || v > 3)
				goto end;

			cli_options.block_index.reader = v;
			break;
		case 'W':
			sscanf(optarg, "%d", &v);
			if (v < 0 || v > 3)
				goto end;

			cli_options.block_index.writer = v;
			break;
		case 'p':
			sscanf(optarg, "%Lc", &v);

			switch (v) {
			case 'b':
				cli_options.writer = writer_blind_write;
				break;
			case 'r':
				cli_options.writer = writer_read_modify_write;
				break;
			default:
				break;
			}
			break;
		case -1:
		default:
			is_done = true;
		}
	}

	if (nb_cpu >= (cli_options.nb_readers + cli_options.nb_writers))
		result = &cli_options;

end:
	return result;
}
