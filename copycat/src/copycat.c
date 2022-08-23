#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// abortWith prints error messages and returns -1 status to the parent process of main ()
void abortWith (char * message, char * extended, char * extVar) {
	if (!extended) fprintf (stderr, "%s\n", message);
	else fprintf (stderr, "ERROR: '%s' %s: %s\n", extVar, message, extended);
	exit (-1);
}

// dataTransaction attempts to read from and write to specified opened files
void dataTransaction (int fdOut, int fdIn, char * fOut, char * fIn, char * buf, long * bufSiz) {
	int rdBytes, wrBytes;
	while ((rdBytes = read (fdIn, buf, *bufSiz)) > 0)
		while (1)
			if ((wrBytes = write (fdOut, buf, rdBytes)) <= 0)
				abortWith ("write() failure", strerror (errno), fOut);
			else if (wrBytes != rdBytes) {
				buf += wrBytes;
				rdBytes -= wrBytes;
			} else break;
	if (rdBytes < 0) abortWith ("read() failure", strerror (errno), fIn);
}

int main (int argc, char ** argv) {
	int fdOut = STDOUT_FILENO, fdIn = -1, bCount = 0, oCount = 0, rdBytes = 0, wrBytes = 0;
	long bufferSize = -1;
	char option = 0, * outputFile = 0;

	// Here, we collect option and option argument information from the argument vector
	while ((option = getopt (argc, argv, "b:o:")) != -1)
		switch (option) {
			case 'b':
				if (++bCount > 1 || !strcmp ("-b", optarg))
					abortWith ("ERROR: Multiple '-b' flags!", 0, 0);
				else if (!strcmp ("-", optarg) || !strcmp ("-o", optarg))
					abortWith ("ERROR: No buffer size specified!", 0, 0);
				else if ((bufferSize = strtol(optarg, NULL, 0)) <= 0)
					abortWith ("ERROR: Invalid buffer size!", 0, 0);
				break;
			case 'o':
				if (++oCount > 1 || !strcmp ("-o", optarg))
					abortWith ("ERROR: Multiple '-o' flags!", 0, 0);
				else if (!strcmp ("-", optarg))
					abortWith ("ERROR: Invalid output file! '-' reserved for stdin; try './-'", 0, 0);
				else if (!strcmp ("-b", optarg))
					abortWith ("ERROR: No output file specified!", 0, 0);
				outputFile = optarg;
				break;
			default:
				abortWith ("Usage: copycat [-b ###] [-o outfile] [infile1 [ infile2 [ ... ]]]", 0, 0);
		}

	// Here, if an output file was specified, we try to open the file; else, we use stdin as output
	if (!outputFile) outputFile = "stdout";
	else if ((fdOut = open (outputFile, O_WRONLY | O_CREAT, 0666)) < 0)
		abortWith ("open() failure [writing]", strerror (errno), outputFile);

	// Here, we declare the dataBuffer with given or default buffer size
	if (bufferSize < 0) bufferSize = 4096;
	char dataBuffer[bufferSize];

	// Here, we take care of opening, reading, & closing input files; and writing output files
	for (; optind < argc; ++optind) {
		if (!strcmp ("-", argv[optind])) {
			fdIn = STDIN_FILENO;
			argv[optind] = "stdin";
		}
		else if ((fdIn = open (argv[optind], O_RDONLY)) < 0)
			abortWith ("open() failure [reading]", strerror (errno), argv[optind]);
		dataTransaction (fdOut, fdIn, outputFile, argv[optind], dataBuffer, &bufferSize);
		if (fdIn != STDIN_FILENO && close (fdIn) < 0)
			abortWith ("close() failure", strerror (errno), argv[optind]);
	}

	// Here, if no input file was specified, we use stdin as the input file
	if (fdIn < 0)
		dataTransaction (fdOut, STDIN_FILENO, outputFile, "stdin", dataBuffer, &bufferSize);

	// Here, we try to close the output file if it isn't stdout
	if (fdOut != STDOUT_FILENO && close (fdOut) < 0)
		abortWith ("close() failure", strerror (errno), outputFile);

	return 0;
}
