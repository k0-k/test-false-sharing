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

enum {
	IDX_READER,
	IDX_WRITER,
};

static struct cli_options cli_options = {
	.nb_loops = NB_LOOPS,
	.threads_group[IDX_READER] = {
		.nb = NB_READERS,
		.block_index = MEM_BLK_READER,
		.entrypoint = thread_entrypoint_reader,
	},
	.threads_group[IDX_WRITER] = {
		.nb = NB_WRITERS,
		.block_index = MEM_BLK_WRITER,
		.entrypoint = thread_entrypoint_writer_read_modify_write,
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
		{"nb_writers", required_argument, NULL, 'w'},
		{"write_pattern", required_argument, NULL, 'p'},
		{"mb_readers", required_argument, NULL, 'R'},
		{"mb_writers", required_argument, NULL, 'W'},
		{"nb_group_a", required_argument, NULL, 'a'},
		{"nb_group_b", required_argument, NULL, 'b'},
		{"mb_group_a", required_argument, NULL, 'A'},
		{"mb_group_b", required_argument, NULL, 'B'},
		{"rw_group_a", required_argument, NULL, '0'},
		{"rw_group_b", required_argument, NULL, '1'},
		{NULL, 0, NULL, 0},
	};
	const struct cli_options *result = NULL;

	for (_Bool is_done = false; is_done != true;) {
		int c = getopt_long(argc, argv, "r:w:l:R:W:p:a:b:A:B:",
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

			cli_options.threads_group[IDX_READER].nb = v;
			break;
		case 'w':
			sscanf(optarg, "%d", &v);
			if (v < 0)
				goto end;

			cli_options.threads_group[IDX_WRITER].nb = v;
			break;
		case 'R':
			sscanf(optarg, "%d", &v);
			if (v < 0 || v > 3)
				goto end;

			cli_options.threads_group[IDX_READER].block_index = v;
			break;
		case 'W':
			sscanf(optarg, "%d", &v);
			if (v < 0 || v > 3)
				goto end;

			cli_options.threads_group[IDX_WRITER].block_index = v;
			break;
		case 'p':
			sscanf(optarg, "%Lc", &v);

			switch (v) {
			case 'b':
				cli_options.threads_group[IDX_WRITER].entrypoint = thread_entrypoint_writer_blind_write;
				break;
			case 'r':
				cli_options.threads_group[IDX_WRITER].entrypoint = thread_entrypoint_writer_read_modify_write;
				break;
			default:
				goto end;
			}
			break;
		case 'a':
			sscanf(optarg, "%d", &v);
			if (v < 0)
				goto end;

			cli_options.threads_group[0].nb = v;
			break;
		case 'b':
			sscanf(optarg, "%d", &v);
			if (v < 0)
				goto end;

			cli_options.threads_group[1].nb = v;
			break;
		case 'A':
			sscanf(optarg, "%d", &v);
			if (v < 0 || v > 3)
				goto end;

			cli_options.threads_group[0].block_index = v;
			break;
		case 'B':
			sscanf(optarg, "%d", &v);
			if (v < 0 || v > 3)
				goto end;

			cli_options.threads_group[1].block_index = v;
			break;
		case '0':
			sscanf(optarg, "%Lc", &v);
			switch (v) {
			case 'r':
				cli_options.threads_group[0].entrypoint = thread_entrypoint_reader;
				break;
			case 'w':
				cli_options.threads_group[0].entrypoint = thread_entrypoint_writer_read_modify_write;
				break;
			case 'b':
				cli_options.threads_group[0].entrypoint = thread_entrypoint_writer_blind_write;
				break;
			default:
				goto end;
			}
			break;
		case '1':
			sscanf(optarg, "%Lc", &v);
			switch (v) {
			case 'r':
				cli_options.threads_group[1].entrypoint = thread_entrypoint_reader;
				break;
			case 'w':
				cli_options.threads_group[1].entrypoint = thread_entrypoint_writer_read_modify_write;
				break;
			case 'b':
				cli_options.threads_group[1].entrypoint = thread_entrypoint_writer_blind_write;
				break;
			default:
				goto end;
			}
			break;
		case -1:
		default:
			is_done = true;
		}
	}

	const int nb_threads0 = cli_options.threads_group[0].nb;
	const int nb_threads1 = cli_options.threads_group[1].nb;
	if (nb_cpu >= (nb_threads0 + nb_threads1))
		result = &cli_options;

end:
	return result;
}
