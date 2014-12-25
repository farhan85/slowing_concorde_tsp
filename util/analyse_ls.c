/**
 *	\file
 *	\brief Allows the user to &lsquo;analyse&rsquo; an L-System.
 *
 *	Given an L-System, this program will create some TSPlib files and run
 *	\e Concorde on those files to compute an upper bound. The user can then
 *	test this upper bound to predict the running times taken by \e Concorde
 *	to solve them.
 *
 *	The user can also find the average running times and number of branch
 *	and bound nodes used by \e Concorde (for any order of the L-System).
 *
 *	\e Concorde is run 15 times on each instance when computing the average
 *	and standard deviation.
 *
 *	\author Farhan Ahammed (faha3615@mail.usyd.edu.au)
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../util/upper_bound.h"
#include "../mt19937ar/t_mt19937ar.h"


#define NUM_TEST_INSTANCES 7
#define NUM_TSP_ITER       15


/**
 *	\brief Plots the lsystem structure (\c ls) with \c plotsize
 *	nodes in the instance, runs \e Concorde on the instance and returns the time
 *	taken to solve it.
 *
 *	\param filename    The TSPlib file to make \e Concorde try to solve.
 *	\param runtime     The running time is returned in this parameter.
 *	\param numbbnodes  The number of branch and bound nodes created is returned
 *	                   in this parameter.
 */
void runtime_bbnodes(char *filename, double *runtime, int *numbbnodes) {
	/* Create and open a pipe to the concorde program. */
	FILE *concorde_pipe;
	char *command;
	asprintf(&command, "concorde %s", filename);
	concorde_pipe = popen(command, "r");
	if (concorde_pipe == NULL)
	{
		fprintf(stderr, "Could not open concorde pipe\n");
		exit(1);
	}

	int nbytes = 810;
	char *concorde_output = (char *)malloc(nbytes + 1), *pos;
	int bytesread;

	/* Run concorde and get the output */
	double rtime = 0;
	int bbnodes = 0;
	bytesread = getdelim(&concorde_output, &nbytes, "\n\n", concorde_pipe);

	/* I could have used my own timer, but the Concorde program already
	   outputs this information. So I decided to use their definition of
	   Concorde's running time.                                          */

	/* If there is no '?' in the output, something went wrong. Since we're
	   only interested in instances that make it work (and are difficult for it)
	   we will return 0 and so it has a weak fitness value.                      */
	pos = strchr(concorde_output, '?');
	if (pos != NULL)
		rtime = strtod(pos+1, NULL);

	pos = strchr(concorde_output, '&');
	if (pos != NULL)
		bbnodes = (int)strtol(pos+1, NULL, 10);

	if (pclose(concorde_pipe) == -1)
	{
		fprintf(stderr, "Could not close concorde pipe\n");
	}
	free(concorde_output);

	if (runtime    != NULL) *runtime    = rtime;
	if (numbbnodes != NULL) *numbbnodes = bbnodes;
}


/**
 *	\brief Obtains the user's input from \c stdin which should be an int.
 *
 *	\return The integer entered by the user.
 */
int getnum()
{
	char text[12];
	int num = 0;
	if (fgets(text, 12, stdin) != NULL )
	{
		char *newline = strchr(text, '\n'); /* search for newline character */
		if (newline != NULL)
		{
			*newline = '\0'; /* overwrite trailing newline */
		}
		num = (int)strtol(text, NULL, 10);
	}
	return num;
}


/**
 *	\brief Computes the square root of the given double.
 *
 *	This code was obtained from: http://www.azillionmonkeys.com/qed/sqroot.html<br>
 *	Last Accessed January 24, 2008.
 *
 *	\param y  The double to compute the square root of.
 *	\return The square root of y.
 */
double fastsqrt (double y) {
	double x, z, tempf;
	unsigned long *tfptr = ((unsigned long *)&tempf) + 1;

	tempf = y;
	*tfptr = (0xbfcdd90a - *tfptr)>>1; /* estimate of 1/sqrt(y) */
	x =  tempf;
	z =  y*0.5;                        /* hoist out the "/2"    */
	x = (1.5*x) - (x*x)*(x*z);         /* iteration formula     */
	x = (1.5*x) - (x*x)*(x*z);
	x = (1.5*x) - (x*x)*(x*z);
	x = (1.5*x) - (x*x)*(x*z);
	x = (1.5*x) - (x*x)*(x*z);
	return x*y;
}



/**
 *	\brief Computes the average running times and number of branch and bound
 *	nodes used when \e Concorde solves the given TSPlib file.
 *
 *	\param filename  The filename of the TSPlib for \e Concorde to solve.
 *	\param avg_rt    The average running time is returned in this parameter.
 *	\param sd_rt     The standard deviation of the population of average runtimes
 *	                 is returned in this parameter.
 *	\param avg_bb    The average running number of branch and bound nodes used
 *	                 is returned in this parameter.
 *	\param sd_bb     The standard deviation of the population of average number of
 *	                 branch and bound nodes used is returned in this parameter.
 *	\param verbose   Set to true (1) to display the running time and the number
 *	                 of branch and bound nodes used for each time \e Concorde is run.
 */
void avg_runtime_bbnodes(char *filename, double *avg_rt, double *sd_rt, double *avg_bb, double *sd_bb, int verbose)
{
	int i, j, bb;
	double rt, n, Vrt, Vbb, delta_rt, delta_bb;
	double _avg_rt, _sd_rt, _avg_bb, _sd_bb;

	n = _avg_rt = _avg_bb = Vrt = Vbb = 0;
	for (j = 0; j < NUM_TSP_ITER; j++)
	{
		runtime_bbnodes(filename, &rt, &bb);
		if (verbose) printf("RT%2d: %0.3f\tBB: %d\n", j+1, rt, bb);
		n++;

		/* This calculation is used to compute the SD for the average running times. */
		delta_rt = rt - _avg_rt;
		_avg_rt += delta_rt/n;
		Vrt += delta_rt*(rt - _avg_rt); /* NOTE: This expression uses the new value of _avg_rt. */

		/* This calculation is used to compute the SD for the number of branch and bound nodes. */
		delta_bb = bb - _avg_bb;
		_avg_bb += delta_bb/n;
		Vbb += delta_bb*(bb - _avg_bb); /* NOTE: This expression uses the new value of _avg_bb. */
	}

	Vrt = Vrt/(n - 1);
	Vbb = Vbb/(n - 1);
	_sd_rt = fastsqrt(Vrt);
	_sd_bb = fastsqrt(Vbb);

	if (avg_rt != NULL) *avg_rt = _avg_rt;
	if (avg_bb != NULL) *avg_bb = _avg_bb;
	if (sd_rt  != NULL) *sd_rt  = _sd_rt;
	if (sd_bb  != NULL) *sd_bb  = _sd_bb;
}



/**
 *	\brief Creates NUM_TEST_INSTANCES TSPlib files representing the L-System
 *	stored in the given file. The files are created in the current directory.
 *	The names of each file are returned in the array \c filenames.
 *
 *	You \e must delete the individual strings and the array itself when finished
 *	with them.
 *
 *	\param filenames     A pointer to an array of char[]. The names given to each file
 *	                     are stored in this array.
 *	\param numnodes      A pointer to an array of int. The size of each corresponding instance
 *	                     is stored in this array.
 *	\param numInstances  The number of instances to create.
 *	\param lsysfile      The file containing the L-System to use, to generate the TSPlib files.
 *	\param lsname        The name of the L-System to use.
 */
void createTSPFiles(char ***filenames, int **numnodes, int numInstances, const char *lsysfile, const char *lsname)
{
	char **fnames  = (char **)malloc(sizeof(char *) * numInstances);
	int *num_nodes =   (int *)malloc(sizeof(int)    * numInstances);

	int i;
	for (i = 0; i < numInstances; i++)
	{
		asprintf(&fnames[i], "plot_%d", i+1);

		/* Compute the x-y coordinates of the instance represented by this L-System. */
		num_nodes[i] = Lsystem(i + 1, lsysfile, lsname, "plots");

		/* Now create a valid TSPlib file. */
		FILE *outfile_stream = fopen(fnames[i], "w");

		/* Check that the file has sucessfully been opened */
		if (outfile_stream == NULL)
		{
			printf("ERROR: File '%s' could not be opened\n", fnames[i]);
			return;
		}

		/* Build the header file */
		fprintf(outfile_stream, "NAME: %s\n", lsname);
		fprintf(outfile_stream, "TYPE: TSP\n");
		fprintf(outfile_stream, "COMMENT: Created by Farhan Ahammed (farhan.ahammed@studentmail.newcastle.edu.au)\n");
		fprintf(outfile_stream, "DIMENSION: %d\n", num_nodes[i]);
		fprintf(outfile_stream, "EDGE_WEIGHT_TYPE: EUC_2D\n");
		fprintf(outfile_stream, "NODE_COORD_TYPE: TWOD_COORDS\n");
		fprintf(outfile_stream, "NODE_COORD_SECTION:\n");

		/* Add the coordinates of the points in this instance. */
		FILE *nodefile = fopen("plots", "r");
		char line[255];
		while(fgets(line, 255, nodefile) != NULL)
		{
			fprintf(outfile_stream, "%s", line);
		}
		fclose(nodefile);
		remove("plots");

		fprintf(outfile_stream, "EOF:\n");

		/* Close the file */
		int close_error_plot = fclose(outfile_stream);
		if (close_error_plot)
		{
			printf("ERROR: File '%s' could not be closed\n", fnames[i]);
		}
	}


	(*filenames) = fnames;
	(*numnodes) = num_nodes;
}

/**
 *	\brief Creates a TSPlib plot of the specified L-System.
 *
 *	You \e must delete the individual strings and the array itself when finished
 *	with them.
 *
 *	\param filename The name to give to the TSPlib file.
 *	\param order    The order of the fractal to create.
 *	\param lsysfile The file containing the L-System to use, to generate the TSPlib files.
 *	\param lsname   The name of the L-System to use.
 */
int createOneTSPFile(char *filename, int order, const char *lsysfile, const char *lsname)
{
	/* Compute the x-y coordinates of the instance represented by this L-System. */
	int numnodes = Lsystem(order, lsysfile, lsname, "plots");

	/* Now create a valid TSPlib file. */
	FILE *outfile_stream = fopen(filename, "w");

	/* Check that the file has sucessfully been opened */
	if (outfile_stream == NULL)
	{
		printf("ERROR: File '%s' could not be opened\n", filename);
		return;
	}

	/* Build the header file */
	fprintf(outfile_stream, "NAME: %s\n", lsname);
	fprintf(outfile_stream, "TYPE: TSP\n");
	fprintf(outfile_stream, "COMMENT: Created by Farhan Ahammed (farhan.ahammed@studentmail.newcastle.edu.au)\n");
	fprintf(outfile_stream, "DIMENSION: %d\n", numnodes);
	fprintf(outfile_stream, "EDGE_WEIGHT_TYPE: EUC_2D\n");
	fprintf(outfile_stream, "NODE_COORD_TYPE: TWOD_COORDS\n");
	fprintf(outfile_stream, "NODE_COORD_SECTION:\n");

	/* Add the coordinates of the points in this instance. */
	FILE *nodefile = fopen("plots", "r");
	char line[255];
	while(fgets(line, 255, nodefile) != NULL)
	{
		fprintf(outfile_stream, "%s", line);
	}
	fclose(nodefile);
	remove("plots");

	fprintf(outfile_stream, "EOF:\n");

	/* Close the file */
	int close_error_plot = fclose(outfile_stream);
	if (close_error_plot)
	{
		printf("ERROR: File '%s' could not be closed\n", filename);
	}

	return numnodes;
}


/**
 *	\brief Reads the specified L-System from the given file and creates
 *	tsplib files of various sizes of the plots represented by the L-System.
 *	An upper bound is computed and the user can test it by creating different
 *	instances and running \e Concorde on them.
 *
 *	\param filename The file containing the L-System.
 *	\param lsname   The L-System to analyse.
 */
void find_fitness(const char *filename, const char *lsname)
{

	int *instancesize;
	char **filenames;
	double avg_rt, sd_rt, avg_bb, sd_bb;

	createTSPFiles(&filenames, &instancesize, NUM_TEST_INSTANCES, filename, lsname);

	/* Create a plot of the running times. */
	int i;
	data_point points[NUM_TEST_INSTANCES];
	printf("Creating instances of different sizes and running Concorde on them\n");
	printf("Order\tInstance Size\tAvg. Run Time\tAvg. BB Nodes\n----------------------------------------------------------------------\n");
	fflush(stdout);
	for (i = 0; i < NUM_TEST_INSTANCES; i++)
	{
		printf(" %d\t %d\t\t", i+1, instancesize[i]);
		fflush(stdout);

		avg_runtime_bbnodes(filenames[i], &avg_rt, &sd_rt, &avg_bb, &sd_bb, 0);

		printf(" %7.4f (%7.4f)\t%7.4f (%7.4f)\n", avg_rt, sd_rt, avg_bb, sd_bb);
		fflush(stdout);

		points[i].x = instancesize[i];
		points[i].y = avg_rt;

		free(filenames[i]);
	}
	free(instancesize);
	free(filenames);
	system("rm -f Oplot* plot*");
	printf("\n");

	/* Compute the upper bound of the running times of the plots of different sizes. */
	double func[NUM_TEST_INSTANCES];

	int offset = 0;
	while ((points[offset].x < 60) || (points[offset].y == 0)) offset++;
	offset--;

	/* We might not like too many (x,0) points, but we need to have at least
	   three points to compute the upper bound. */
	if (NUM_TEST_INSTANCES - offset < 3) offset = NUM_TEST_INSTANCES - 3;

	find_upper_bound(func, points+offset, NUM_TEST_INSTANCES-offset);
	printf("data points bounded above by:\n\ty =  %0.4f + %0.4f((x - %0.4f)/%0.4f)^(%0.6f)\n", func[2], func[4], func[1], func[3], func[0]);
	printf("\n");

	/* Compute the sum of errors. */
	double se = sum_error(func, points+offset, NUM_TEST_INSTANCES-offset);
	printf("SE: %f\n", se);

	printf("Fitness: %0.4f^2/%0.4f = %0.4f", func[0], se+1, (func[0]*func[0])/(se+1));

	int order, instsize;
	double pred;
	char response;
	int finished = 0;

	do
	{
		printf("\nEnter an order: ");
		order = getnum();

		instsize = createOneTSPFile("tempfile", order, filename, lsname);
		pred = func[2] + func[4]*pow((instsize - func[1])/func[3], func[0]);
		printf("Prediction: %0.4fs\n", pred);

		printf("\nPress enter to run Concorde\n");
		getchar();
		printf("\n");
		system("concorde tempfile");

		printf("\nRun again? ");
		response = getchar();
		finished = ((response != 'y') && (response != 'Y'))	;

		/* For some reason, fflush(stdin) doesn't work. I need to use getchar() to
		   remove the '\n'. */
		while (getchar() != '\n') {}
	} while (!finished);
	remove("tempfile");
}



void find_runtime(const char *filename, const char *lsname)
{
	int order, numcities;
	char text[4], *tspname = "temp.tsp";
	double avg_rt, sd_rt, avg_bb, sd_bb;


	printf("Which order do you want to test? ");
	order = getnum();

	printf("order: %d\n", order); fflush(stdout);

	numcities = createOneTSPFile(tspname, order, filename, lsname);
	printf("Number of Cities: %d\n", numcities); fflush(stdout);

	avg_runtime_bbnodes(tspname, &avg_rt, &sd_rt, &avg_bb, &sd_bb, 1);
	system("mv *.sol solution");
	system("rm *plot* *temp*");
	printf("\n");

	printf("Running Times:\n");
	printf("Average: %0.4f\n", avg_rt);
	printf("St. Dev: %0.4f\n",  sd_rt);
	printf("68%% lies within:  (%7.4f, %7.4f)\n", avg_rt -   sd_rt, avg_rt +   sd_rt);
	printf("95%% lies within:  (%7.4f, %7.4f)\n", avg_rt - 2*sd_rt, avg_rt + 2*sd_rt);
	printf("\n");

	printf("Number of Branch and Bound Nodes:\n");
	printf("Average: %0.4f\n", avg_bb);
	printf("St. Dev: %0.4f\n",  sd_bb);
	printf("68%% lies within:  (%7.4f, %7.4f)\n", avg_bb -   sd_bb, avg_bb +   sd_bb);
	printf("95%% lies within:  (%7.4f, %7.4f)\n", avg_bb - 2*sd_bb, avg_bb + 2*sd_bb);
}

/**
 *	\brief Starts the program.
 */
int main(int argc, char** argv)
{
	char text[4];
	int choice;

	if (argc != 3)
	{
		printf("usage:\n%s <L-System file> <L-System name>", argv[0]);
		return 0;
	}

	printf("Do you want to:\n");
	printf("[1] Find an upper bound and compute the fitness, or\n");
	printf("[2] Find the average running time for an instance size, or\n");
	printf("[3] Exit?\n");
	fflush(stdout);

	choice = getnum();

	int finished = 0, ch;
	char response;
	switch (choice)
	{
		case 1:
			find_fitness(argv[1], argv[2]);
			break;
		case 2:

			do
			{
				find_runtime(argv[1], argv[2]);

				printf("\nRun again? ");
				response = getchar();
				finished = ((response != 'y') && (response != 'Y'));

				while (((ch = getchar()) != EOF) && ((char)ch != '\n'));
			} while (!finished);
			break;
	}

	return 0;
}
