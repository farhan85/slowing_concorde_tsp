/**
 *	\file
 *  \brief Uses the evolutionary algorithm technique to find a fractal
 *	(represented by an L-System) such that the actual <i>(x,y)</i>
 *	data points are used as an instance of a Travelling Salesman Problem
 *	solved by the program \e Concorde and are relatively difficult to solve.
 *
 *	Concorde web site: http://www.tsp.gatech.edu/concorde/index.html
 *
 *	In order for this program to work you will need to obtain the source code for
 *	Concorde and replace the file <c>concorde.c</c> with mine (if it is not distributed
 *	with this file, check my website http://www.eng.newcastle.edu.au/~c3004154)
 *	then run \c make to compile and create the \e Concorde program.
 *
 *	This program runs \e Concorde by executing the command \c concorde. Make
 *	sure that \c concorde can be found in <c>$PATH</c>.
 *
 *	\author Farhan Ahammed (faha3615@mail.usyd.edu.au)
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../ls/ls_pop.h"

/**
 *	\brief The actual data-type of the elements in the population.
 */
typedef ls_population datatype;


static void seed_population(ls_population *pop, lsystem **temp, int n, int initial_rule_len)
{
	/* Note: Because every population must have the same number of rules, n say:
	          - If a user-given L-System has too many rules, we take only the first n rules.
	          - If a user-given L-System has too few rules, we add some random ones.

	   Also, if there are more user-given L-Systems, m say, than can be fit in the
	   population, we add only the first m L-Systems.
	*/

	int i, j, m;
	char curr_symbol_name;
	m = (pop->pop_size < n) ? (pop->pop_size) : n;

	for (i = 0; i < m; i++)
	{
		delete_ls(pop->ls_array[i]);
		pop->ls_array[i] = temp[i];

		/* Add extra rules or remove existing ones if necessary. */
		if (temp[i]->numrules != pop->numrules)
		{
			temp[i]->rule =       (ruleobj **)realloc(temp[i]->rule,       sizeof(ruleobj *) * pop->numrules);
			temp[i]->rulelength = (int *)     realloc(temp[i]->rulelength, sizeof(int)       * pop->numrules);

			if (temp[i]->numrules < pop->numrules)
			{
				if (temp[i]->numrules != 0)
					curr_symbol_name = temp[i]->startvar[temp[i]->numrules - 2];
				else
					curr_symbol_name = 0x40; /* The ASCII character before 'A' */

				/* Create any additional rules. */
				for (j = temp[i]->numrules; j < pop->numrules; j++)
				{
					temp[i]->rule[j] = (ruleobj *)malloc(sizeof(ruleobj) * initial_rule_len);
					temp[i]->rulelength[j] = initial_rule_len;

					curr_symbol_name++;
					if ((curr_symbol_name == 'D') || (curr_symbol_name == 'M'))
						curr_symbol_name++;
					else if (curr_symbol_name == 'F')
						curr_symbol_name = curr_symbol_name + 2;

					temp[i]->startvar[j-1] = curr_symbol_name;
				}
				/* Now that we know the names of all the rules, we can set random rules. */
				for (j = temp[i]->numrules; j < pop->numrules; j++)
				{
					random_rule(temp[i], j);
				}
				temp[i]->numrules = pop->numrules;
			}
			temp[i]->numrules = pop->numrules;
		}
	}
}



/**
 *  \brief Starts the program.
 *
 *	<i>Arguments are expected.</i> Run the program like this:\n
 *	<c>&lt;this program&gt; &lt;\#functions in each pop&gt; &lt;\#parents&gt; &lt;\#generations&gt;</c>\n
 *
 *	All arguments \e must be integers.\n
 *	<c>&lt;\#generations&gt;</c> can accept a \c long number.
 *
 *	\param argc The size of the array \c argv.
 *	\param argv The array of arguments passed to this program.
 *	\return \c 0 If this program executed properly.
 */
int main(int argc, char** argv)
{
	int num_rules = 0, numparents = 0, rule_size = 10, display_data = 0, verbose = 0;
	long number_of_generations = 0;

	/* Check if the user gave any of the option arguments. */
	int c;
	/* The special significance of a colon as the first character of optstring
       makes getopt() consistent with the getopts utility. It allows an application
       to make a distinction between a missing argument and an incorrect option
       letter without having to examine the option letter. It is true that a
       missing argument can only be detected in one case, but that is a case that
       has to be considered.

       This description was obtained from
                     http://www.frech.ch/man/man3p/optopt.3p.html
       (Last Accessed July 15, 2007)                                                */
	while ((c = getopt (argc, argv, ":dhv")) != -1)
	{
		switch (c)
		{
			case 'd':
				display_data = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'h':
				fprintf(stderr, "Usage: %s [-OPTION] [<#rules per L-System> <initial rule length> <#parents> <#generations>]\n", argv[0]);
				fprintf(stderr, " -d \t Displays the actual population after each operation.\n");
				fprintf(stderr, " -v \t Verbose mode. Displays each step this program takes.\n");
				fprintf(stderr, " -h \t Displays this help and exits.\n");
				return 1;
			case '?':
				fprintf(stderr, "Ignoring unrecognized option: -%c\n", optopt);
		}
	}


	/* There must be four non-option arguments. */
	if ((argc - optind) < 4)
	{
		fprintf(stderr, "ERROR: Expected at least three arguments.\n");
		fprintf(stderr, "Usage: %s [-d] [-v] [<#rules per L-System> <initial rule length> <#parents> <#generations>]\n", argv[0]);
		fprintf(stderr, " -d \t To display the actual population after each operation.\n");
		fprintf(stderr, " -v \t Verbose mode. Displays each step this program takes.\n");
		return 1;
	}
	else
	{
		int index = optind;
		num_rules = (int)strtol(argv[index], NULL, 10);
		rule_size = (int)strtol(argv[index + 1], NULL, 10);
		numparents = (int)strtol(argv[index + 2], NULL, 10);
		number_of_generations = strtol(argv[index + 3], NULL, 10);
	}


	/* The variables needed for predicting the completion times. */
	time_t start_time = time (NULL), curtime, predict_finish, predict_nextpercent;
	double elapsedtime, time_interval;
	struct tm *predict_finish_tm, *predict_nextpercent_tm;

	long count = 0;
	int display_progress = 0;

	/* Find out when we will reach 10%, 20%, ..., 100% so that we can display
	   them on the screen */
	char *percent_str[10];
	int percent_value[10];
	double percent_value_fitness[10];
	int i;
	for (i = 0; i < 10; i++) {
		asprintf(&percent_str[i], "%i0%% complete", i+1);
		percent_value[i] = (int)((double)number_of_generations*((double)(i+1)/10));
		percent_value_fitness[i] = 0;
	}


	/* Let's begin. Create a random population to start with. */
	datatype *pop;
	if (verbose) { printf("Creating population..."); fflush(stdout); }
	createPopulation(&pop, numparents, num_rules, rule_size);
	if (verbose) { printf("done.\n"); }

	if (verbose) { printf("Assigning Random values..."); fflush(stdout); }
	assignRandomValues(pop);
	if (verbose) { printf("done.\n"); }

	/* Check if we should seed the population. */
	if (verbose) { printf("Should we seed the population?..."); fflush(stdout); }
	FILE *seedfp = fopen("seed.txt", "r");
	if (seedfp != NULL)
	{
		if (verbose) { printf("yes.\n"); }
		if (verbose) { printf("Adding L-Systems to the population..."); fflush(stdout); }
		int n;
		lsystem **temp;
		n = readfile(seedfp, &temp, NULL);

		if (n > 0) seed_population(pop, temp, n, rule_size);
		if (fclose(seedfp)) printf("ERROR: File 'seed.txt' could not be closed\n");
		if (verbose) { printf("done.\n"); }

	}
	else
	{
		if (verbose) { printf("no.\n"); }
	}
	if (display_data)
	{
		printf("Initial Population\n");
		printall(pop);
	}

	/* Sort the population so that we can find the best individuals to be parents. */
	sortpopulation(pop);

	while (++count <= number_of_generations)
	{
		/* Select the best individuals to be parents and generate the children. */
		if (verbose) { printf("Generating children..."); fflush(stdout); }
		generateChildren(pop);
		if (verbose) { printf("done.\n"); }
		if (display_data)
		{
			printf("Generated Children\n");
			printall(pop);
		}

		/* Mutate the resulting offspring. */
		if (verbose) { printf("Mutating children..."); fflush(stdout); }
		mutateChildren(pop);
		if (verbose) { printf("done.\n"); }
		if (display_data)
		{
			printf("Mutated Children\n");
			printall(pop);
		}

		/* Sort the population so that we can find the best individuals to be parents. */
		sortpopulation(pop);

		/* If we've progressed through a further 10% of the generations/iterations, display
		   this information on screen. Also display how good (fit) is the best individual
		   (datatype) so far.                                                               */
		if (count == percent_value[display_progress])
		{
			/* We'd like to know how the population improves. */
			percent_value_fitness[display_progress] = getfitness(pop, 0);

			/* Show the best datatype we've got so far. */
			printf("%s\n", percent_str[display_progress]);
			printbest(pop);

			/* Print out the predicted completion times. */
			curtime = time(NULL);
			elapsedtime = difftime(curtime, start_time);
			time_interval = (elapsedtime/(display_progress+1));

			predict_finish = start_time + time_interval*10;
			predict_finish_tm = localtime(&predict_finish);
			printf("Estimated finish time: %d:%02d:%02d\n",
				predict_finish_tm->tm_hour,
				predict_finish_tm->tm_min,
				predict_finish_tm->tm_sec);

			if (display_progress < 9)
			{
				predict_nextpercent = curtime + time_interval;
				predict_nextpercent_tm = localtime(&predict_nextpercent);
				printf("Estimated time when %s: %d:%02d:%02d\n\n\n",
					percent_str[display_progress+1],
					predict_nextpercent_tm->tm_hour,
					predict_nextpercent_tm->tm_min,
					predict_nextpercent_tm->tm_sec);
			}
			else
				printf("\n\n\n");

			display_progress++;
			/* TODO: Implement the renewpop(ifs) method.
			if (display_progress%3 == 0)
			{
				fprintf(progress_stm, "Renewing Population...");
				renewpop(pop);
				fprintf(progress_stm, "done.\n");
			}
			*/
		}
	}

	/* Show the user the best individual we have found. */
	printf("Best individual Found:\n");
	printbest(pop);

	/* Save the whole population to files */
	/* First create the folder. */
	if (verbose) { printf("Constructing directory..."); fflush(stdout); }
	curtime = time(NULL);
	struct tm *localtm = localtime(&curtime);
	char str_time[13];
	int n = strftime(str_time, 13, "%d%m%y_%H%M", localtm);
	mkdir(str_time, S_IRWXU);
	if (verbose) { printf("done. Created ./%s/\n", str_time); }

	char *fname, *lsname;

	// Save information about all the instances to file
	for (i = 0; i < pop->pop_size; i++)
	{
		// To make it easier for us when looking for the instances, the filename will contain
		// the time it was created and the fitness of the instance it contains.
		if (verbose) { printf("Constructing filename..."); fflush(stdout); }
		asprintf(&fname, "./%s/%0.4f.ls", str_time, getfitness(pop, i));
		if (verbose) { printf("done.\n"); }

		// Add more detailed information about the fitness
		if (verbose) { printf("Constructing extra information to put in file..."); fflush(stdout); }
		char *extra_info;
		asprintf(&extra_info, "\nImprovement of Fitness:\n %f %f %f %f %f %f %f %f %f %f \n",
			percent_value_fitness[0],
			percent_value_fitness[1],
			percent_value_fitness[2],
			percent_value_fitness[3],
			percent_value_fitness[4],
			percent_value_fitness[5],
			percent_value_fitness[6],
			percent_value_fitness[7],
			percent_value_fitness[8],
			percent_value_fitness[9]);
		if (verbose) { printf("done.\n"); }

		// Save to file
		asprintf(&lsname, "LS%d", i);
		if (verbose) { printf("Saving L-System (%s) to file (%s)...", lsname, fname); fflush(stdout); }
		savelstofile(pop, i, fname, lsname, extra_info);
		if (verbose) { printf("done.\n"); }
	}

	/* Free up any memory. */
	if (verbose) { printf("Deleting population from memory..."); fflush(stdout); }
	deletePopulation(pop);
	if (verbose) { printf("done.\n"); }

	return 0;
}
