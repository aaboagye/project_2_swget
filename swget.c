#include <stdio.h>
#include <stdlib.h>
#include <argp.h>

const char *argp_program_version = "swget 1.0";
const char *argp_program_bug_address = "<aseda.aboagye@gmail.com>";
static char doc[] = "swget -- A simple web download utility";
static char args_doc[] = "";		/* No default args */

static struct argp_option options[] = {
	{"url",        'u', "URL",      0,  "URL of object to download", 0 },
	{"destdir",    'd', "DESTDIR",  0,  "Destination directory to save files to", 0 },
	{"verbose",    'v', 0,          0,  "Provide verbose output, including server headers", 0 },
	{ 0, 0, 0, 0, 0, 0} // Last entry should be all zeros in all fields
};

struct arguments {
	int verbose;
	char *url;
	char *destdir;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state){
	/* Get the input argument from argp_parse, which we
	 know is a pointer to our arguments structure. */
	struct arguments *arguments = state->input;

	// Figure out which option we are parsing, and decide
	// how to store it
	switch (key){
		case 'v':
			arguments->verbose = 1;
			break;
		case 'u':
			arguments->url = arg;
			break;
		case 'd':
			arguments->destdir = arg;
			break;
		case ARGP_KEY_END:
			// Check if our url and destdir REQUIRED "options" have been set to non-default values
			if (strcmp(arguments->url, "") ==0 || strcmp(arguments->destdir, "") == 0)
				argp_usage (state);
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char const *argv[]){
	struct arguments arguments;
	arguments.verbose = 0;
	arguments.url = "";
	arguments.destdir = "";
	argp_parse (&argp, argc, argv, 0, 0, &arguments);
	return 0;
}
