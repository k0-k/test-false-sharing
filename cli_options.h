#pragma once

struct threads_config {
	int nb, block_index;
	void *(*entrypoint)(void *args);
};

struct cli_options {
	int nb_loops;
	union {
		struct {
			struct threads_config readers, writers;
		} r_w;
		struct {
			struct threads_config group[2];
		} group;
	} threads_config;
};

const struct cli_options *parse(int argc, char *argv[]);
