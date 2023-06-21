#pragma once

struct threads_config {
	int nb, block_index;
	void *(*entrypoint)(void *args);
};

struct cli_options {
	int nb_loops;
	struct threads_config threads_group[2];
};

const struct cli_options *parse(int argc, char *argv[]);
