#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int redirect (const char * path, int fdR, int options, mode_t mode) {
	int fd;

	if ((fd = open (path, options, mode)) > 0) {
		if (dup2 (fd, fdR) < 0) {
			fprintf (stderr, "ERROR: fd=%d dup2() failure: %s\n", fd, strerror (errno));
			return 1;
		} else if (close (fd) < 0) {
			fprintf (stderr, "ERROR: '%s' close() failure: %s\n", path, strerror (errno));
			return 1;
		}
	} else {
		fprintf (stderr, "ERROR: '%s' open() failure: %s\n", path, strerror (errno));
		return 1;
	}
	return 0;
}

int runCmd (char * cmd, int * eFlag) {
	int pid, exitStatus, waitStatus, timediff, i, j,
		fdErr, fdOut, fdIn;
	struct rusage usage;
	struct timeval t1, t2;
	char * childArgv[1024];
	char * rIn = NULL, * rOut = NULL, * rErr = NULL,
		* rOutApp = NULL, * rErrApp = NULL, * temp;

	i = 0;
	cmd[strlen (cmd) - 1] = 0; // remove \n
	temp = strtok (cmd, " ");

	while (temp != NULL) {
		if (strstr (temp, "2>>") - temp == 0)     rErrApp = temp + 3;
		else if (strstr (temp, ">>") - temp == 0) rOutApp = temp + 2;
		else if (strstr (temp, "2>") - temp == 0) rErr = temp + 2;
		else if (strstr (temp, ">") - temp == 0)  rOut = temp + 1;
		else if (strstr (temp, "<") - temp == 0)  rIn = temp + 1;
		else childArgv[i++] = temp;
		temp = strtok (NULL, " ");	
	}
	childArgv[i] = NULL;
	
	if (!strcmp (childArgv[0], "cd")) {
		if (childArgv[1] == NULL && chdir (getenv ("HOME")) < 0 || childArgv[1] != NULL && chdir (childArgv[1]) < 0) {
			fprintf (stderr, "ERROR: chdir() failure: %s\n", strerror (errno));
			*eFlag = 1;
			return 1;
		}
		return 0;
	}

	if (gettimeofday(&t1, NULL) < 0) {
		fprintf (stderr, "ERROR: gettimeofday() failure: %s\n", strerror (errno));
		*eFlag = 1;
		return 1;
	}

	switch (pid = fork ()) {
		case -1:
			fprintf (stderr, "ERROR: '%s' fork() failure: %s\n", childArgv[0], strerror (errno));
			*eFlag = 1;
			return 1;
		case 0:
			// handle redirection
			if (rErrApp != NULL) {
				if (redirect (rErrApp, 2, O_RDWR | O_APPEND | O_CREAT, 0666)) exit (1);
			} else if (rErr != NULL)
				if (redirect (rErr, 2, O_RDWR | O_TRUNC | O_CREAT, 0666)) exit (1);
			if (rOutApp != NULL) {
				if (redirect (rOutApp, 1, O_RDWR | O_APPEND | O_CREAT, 0666)) exit (1);
			} else if (rOut != NULL)
				if (redirect (rOut, 1, O_RDWR | O_TRUNC | O_CREAT, 0666)) exit (1);
			if (rIn != NULL && redirect (rIn, 0, O_RDONLY, 0666)) exit (1);

			if (execvp (childArgv[0], childArgv) < 0) {
				fprintf (stderr, "ERROR: '%s' execvp() failure: %s\n", childArgv[0], strerror (errno));
				exit (1);
			}
		default:
			if (wait4 (pid, &waitStatus, 0, &usage) > 0) {
				if ((exitStatus = WEXITSTATUS (waitStatus)) != 0) *eFlag = 1;
				if (gettimeofday(&t2, NULL) < 0) {
					fprintf (stderr, "ERROR: gettimeofday() failure: %s\n", strerror (errno));
					*eFlag = 1;
				}
				timediff = t2.tv_sec * 1000000 + t2.tv_usec - (t1.tv_sec * 1000000 + t1.tv_usec);

				fprintf (stderr, "\n[COMMAND INFO]\n   Return Code:\t%d\n", exitStatus);
				fprintf (stderr, "   Real Time:\t%d.%04ds\n", timediff / 1000000, timediff % 1000000);
				fprintf (stderr, "   System Time:\t%d.%04ds\n", usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
				fprintf (stderr, "   User Time:\t%d.%04ds\n", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
			} else fprintf (stderr, "ERROR: pid=%d wait4() failure: %s\n", pid, strerror (errno));
	}

	return 0;
}

void printPrompt () {
	char cwd[1024];
	if (!getcwd (cwd, sizeof (cwd)))
		fprintf (stderr, "ERROR: getcwd() failure: %s\n", strerror (errno));
	printf ("%s $ ", cwd);
}

int main (int argc, char ** argv) {
	int eFlag, bytesRead;
	size_t cmdLength = 1024;
	char * cmd = NULL;
	FILE * inFile;

	eFlag = 0;
	
	if (argc > 1 && (inFile = fopen (argv[1], "r")) == NULL) {
		fprintf (stderr, "ERROR: '%s' fopen() failure: %s\n", argv[1], strerror (errno));
		return 1;
	} else if (argc == 1) inFile = stdin;

	if (inFile == stdin) printPrompt ();

	while ((bytesRead = getline (&cmd, &cmdLength, inFile)) > 0) {
		if (bytesRead <= 1 || cmd[0] == '#' || cmd[bytesRead - 1] != '\n') {
			errno = 0;
			if (inFile == stdin) printPrompt ();
			continue;
		} else runCmd (cmd, &eFlag);

		if (inFile == stdin) printPrompt ();
		errno = 0;
	}

	if (errno != 0) {
		fprintf (stderr, "\nERROR: getline() failure: %s\n", strerror (errno));
		fprintf (stderr, "Aborting...\n");
		return 1;
	} else printf ("\nExiting shell: EOF reached. Goodbye!\n");
		
	return eFlag;
}
