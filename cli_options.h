#pragma once

struct cli_options {
	void *(*writer)(void *args);
	int nb_loops;
	int nb_readers;
	int nb_writers;
	struct {
		int reader, writer;
	} block_index;
};

const struct cli_options *parse(int argc, char *argv[]);
