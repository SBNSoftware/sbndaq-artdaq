/*
 * Test program for the ambient capabilities. This program spawns a shell
 * that allows running processes with a defined set of capabilities.
 *
 * (C) 2015 Christoph Lameter
 * (C) 2015 Serge Hallyn
 * Released under: GPL v3 or later.
 *
 * Tweaks made by Ron Rechenmacher, March, 2020.
 *
 * Compile using:
 *
 * gcc -Wall -o ambient_exec{,.c} -lcap-ng
 *
 * This program must have the following capabilities to run properly:
 * Permissions for CAP_NET_RAW
 *
 * A command to equip the binary with the right caps is:
 *
 * setcap cap_net_raw+p ambient_exec
 *
 *
 * To get a shell with additional caps that can be inherited by other processes:
 *
 * ./ambient_exec /bin/bash
 *
 *
 * Verifying that it works:
 *
 * From the bash spawed by ambient_exec run
 *
 * grep Cap /proc/$$/status
 *
 * Or, more directly: ./ambient_exec /bin/grep Cap /proc/self/status
 *
 * $RCSfile: ambient_exec.c,v $
 */
static const char *rev="$Revision: 1.4 $$Date: 2020/03/16 20:27:24 $";

#include <sys/prctl.h>			/* prctl */
#include <cap-ng.h>	/* capng_get_caps_process; yum install libcap-ng-devel */
#include <stdio.h>	/* perror, printf */
#include <stdlib.h>	/* exit */
#include <string.h>	/* strdup, strtok, strcmp */


/*
 * Definitions from the kernel header files. These are going to be removed
 * when the /usr/include files have these defined.
 */
#define PR_CAP_AMBIENT 47
#define PR_CAP_AMBIENT_IS_SET 1
#define PR_CAP_AMBIENT_RAISE 2
#define PR_CAP_AMBIENT_LOWER 3
#define PR_CAP_AMBIENT_CLEAR_ALL 4

static void set_ambient_cap(int cap)
{
	int rc;

	capng_get_caps_process();
	rc = capng_update(CAPNG_ADD, CAPNG_INHERITABLE, cap);
	if (rc) {
		printf("Cannot add inheritable cap\n");
		exit(2);
	}
	capng_apply(CAPNG_SELECT_CAPS);

	/* Note the two 0s at the end. Kernel checks for these */
	if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0)) {
		perror("Cannot set cap");
		exit(1);
	}
}

void usage(const char *me) {
	printf("Usage: %s [-c caps] new-program new-args\n%s\n", me, rev);
	exit(1);
}

//int default_caplist[] = {CAP_NET_RAW, CAP_NET_ADMIN, CAP_SYS_NICE, -1};
int default_caplist[] = {CAP_NET_RAW, -1};

int *get_caplist(const char *arg) {
	int i = 1;
	int *list = NULL;
	char *dup = strdup(arg), *tok;

	for (tok = strtok(dup, ","); tok; tok = strtok(NULL, ",")) {
		list = realloc(list, (i + 1) * sizeof(int));
		if (!list) {
			perror("out of memory");
			exit(1);
		}
		list[i-1] = atoi(tok);
		list[i] = -1;
		i++;
	}
	return list;
}


int main(int argc, char **argv)
{
	int i;
	int *caplist = NULL;
	int index = 1; // argv index for cmd to start

	if (argc < 2)
		usage(argv[0]);

	if (strcmp(argv[1], "-c") == 0) {
		if (argc <= 3) {
			usage(argv[0]);
		}
		caplist = get_caplist(argv[2]);
		index = 3;
	}

	if (!caplist) {
		caplist = (int *)default_caplist;
	}

	for (i = 0; caplist[i] != -1; i++) {
		printf("adding %d to ambient list\n", caplist[i]);
		set_ambient_cap(caplist[i]);
	}

	printf("ambient_exec forking shell\n");
	if (execv(argv[index], argv + index))
		perror("Cannot exec");

	return 0;
}
