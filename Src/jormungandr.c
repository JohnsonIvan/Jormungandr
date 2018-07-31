/*
 * Src/jormungandr.c
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */

#include <argp.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "job.h"
#include "jormungandr.h"
#include "messenger.h"
#include "server.h"

#define OPTION_PRIORITY 'p'
#define OPTION_NUMSLOTS 's'

const char *argp_program_version =
	"Jörmungandr v0.1.0\n"
	"Copyright(C) 2018, Ivan Tobias Johnson\n"
	"License GPLv2.0: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html\n"
	"This software comes with no warranty, to the extent permitted by applicable law";
const char *argp_program_bug_address = "<git@IvanJohnson.net>";
static const char doc[] = "Jörmungandr -- a tool running a queue of jobs";
static const char args_doc[] =
	"launch <serverdir> [-s numslots] [--numslots=numslots]\n"
	"schedule <serverdir> [-p] [--priority] -- <cmd> [args...]";
static struct argp_option options[] = {
	{"numslots", OPTION_NUMSLOTS, "numslots", OPTION_NO_USAGE, "Specifies the number of slots to start servers with", 0},
	{"priority", OPTION_PRIORITY, 0,          OPTION_NO_USAGE, "Put the given command at the front of the queue",     0},
	{0,0,0,0,0,0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	char *end;
	long val;

	switch (key) {
	case ARGP_KEY_INIT:
		arguments->task = task_undefined;
		arguments->server = NULL;
		arguments->cmd = NULL;
		arguments->numSlots = 0;
		arguments->priority = false;
		break;
	case OPTION_PRIORITY:
		arguments->priority = true;
		break;
	case OPTION_NUMSLOTS:
		val = strtol(arg, &end, 10); //base 10
		if (end[0] != '\0') {
			argp_usage(state); //no return
		}
		if (val < 0 || val > UINT_MAX) {
			printf("The number of slots must be in the range [%u, %u]\n",
				0, UINT_MAX);
			argp_usage(state); //no return
		}
		arguments->numSlots = (unsigned int) val;
		break;
	case ARGP_KEY_ARG:
		if (state->arg_num == 0) {
			if (strcmp(arg, "launch") == 0) {
				arguments->task = task_launch;
			} else if (strcmp(arg, "schedule") == 0) {
				arguments->task = task_schedule;
			} else {
				argp_usage(state);
			}
		} else if (state->arg_num == 1) {
			arguments->server = arg;
		} else {
			return ARGP_ERR_UNKNOWN;
		}
		break;
	case ARGP_KEY_ARGS:
		arguments->cmd      = state->argv + state->next;
		arguments->cmdCount = state->argc - state->next;
		break;
	case ARGP_KEY_END:
		switch (arguments->task) {
		case task_schedule:
			if (state->arg_num <= 2) {
				argp_usage(state);
			}
			break;
		case task_launch:
			if (state->arg_num != 2) {
				argp_usage(state);
			}
			break;
		default:
			argp_usage(state);
		}
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

struct arguments parseArgs(int argc, char **argv)
{
	struct arguments args;
	args.server = NULL;
	args.cmd = NULL;
	argp_parse(&argp, argc, argv, 0, 0, &args);
	return args;
}

int fulfilArgs(struct arguments args)
{
	int server = getServerDir(args.server);
	if (server < 0) {
		puts("Could not get serverdir");
		return 1;
	}

	int fail;
	struct job job;
	switch (args.task) {
	case task_launch:
		return messengerLaunchServer(server, args.numSlots);
	case task_schedule:
		job.argc = args.cmdCount;
		job.argv = args.cmd;
		job.priority = args.priority;

		fail = messengerSendJob(server, job);
		if (fail) {
			puts("Failed to send job");
		} else {
			puts("Sent job");
		}
		return 0;
	case task_undefined:
		exit(1);
	default:
		exit(1);
	}
}

#ifndef TEST
int main(int argc, char **argv)
{
	struct arguments args = parseArgs(argc, argv);
	return fulfilArgs(args);
}
#endif
