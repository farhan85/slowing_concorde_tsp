/**
 *	\file
 *	\brief Implements the methods defined in the file ls.h.
 *
 *	\author Farhan Ahammed (faha3615@mail.usyd.edu.au)
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ls.h"
#include "../util/upper_bound.h"
#include "../mt19937ar/t_mt19937ar.h"


/**
 *	\brief The random number generator we'll use (The Mersenne Twister Pseudo
 *	Random Number Generator).
 */
mt_prng rng;

#define NUM_TSP_ITER   3    /* The number of times to run Concorde on an instance when computing the AVERAGE fitness. */


/**
 *	\brief Concatenates the string \c src onto the string \c dest and
 *	returns a char pointer to the position of the terminationg \c null
 *	character in \c dest. This allows for more efficient use of this
 *	function.
 *
 *	\param  dest The string to concatenate onto.
 *	\param  src  The string to copy from.
 *	\return A char pointer to the position of the terminationg \c null
 *	        character in \c dest.
 */
char* mystrcat(char* dest, char* src)
{
	while (*dest++ = *src++);
	return --dest;
}


/**
 *	\brief Converts an array of ruleobj into a readable string.
 *
 *	Note: You \e must \c free() the returned string.
 *
 *	\param  rule       A ruleobj array representation of a rule.
 *	\param  rulelength The size of the ruleobj array.
 *	\return A string version of the rule.
 */
char *rule2string(ruleobj *rule, int rulelength)
{
	int temp = 0, i;
	for (i = 0; i < rulelength; i++)
		temp += rule[i].len;

	char *str = (char *)malloc(temp + 1);
	char *_str = str;

	for (i = 0; i < rulelength; i++)
	{
		_str = mystrcat(_str, rule[i].str);
	}
	str[temp] = '\0';

	return str;
}


/**
 *	\brief Computes the square root of the given double.
 *
 *	This code was obtained from: http://www.azillionmonkeys.com/qed/sqroot.html
 * 	Last Accessed January 24, 2008.
 *
 *	\param  y The number to compute the square root of.
 *	\return The square root of \c y.
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
 *	\brief Extracts the immediate number from the L-System rule.
 *
 *	For example, if <c>str = "I1.36F@4.5"</c> then this function will return
 *	<c>"I1.36"</c> (which represents the inverse of 1.36).
 *
 *	\param str         The string to extract the number from.
 *	\param result      The string to store the extracted number to.
 *	\param result_size The length of \c result. If \c result is not long
 *	                   enough, the whole number won't be copied.
 *	\return The length of the string written (\e including the \c null).
 */
int extract_number(char *str, char *result, int result_size)
{
	int n = 0;

	while ( (n < result_size-1) &&
	        ( ((*str <= '9') && (*str >= '0')) || (*str == '.') || (*str == 'Q') || (*str == 'I'))
	      )
	{
		result[n] = *str;
		n++;
		str++;
	}
	result[n] = '\0';
	return n;
}


/**
 *	\brief Convert an string representation of an L-System rule into a ruleobj
 *	object.
 *
 *	Note: You \e must \c free() the returned ruleobj array.
 *
 *	\param  s      A string representation of a rule.
 *	\param  rulelength The size of the returned ruleobj array is stored in the
 *	                   int pointed to by \c rulelength.
 *	\return A ruleobj array version of the rule.
 */
ruleobj *string2rule(char *s, int *rulelength)
{
	ruleobj *rule, *temprule, *ret_rule;
	double num;
	char numstr[7];
	int count = 0, s_len, temp_num;

	s_len = strlen(s);
	rule = (ruleobj *)malloc(sizeof(ruleobj)*s_len);
	temprule = rule;

	while (*s)
	{
		if (*s == 'F')
		{
			temprule->str[0] = 'F';
			temprule->str[1] = '\0';
			temprule->len = 1;
			temprule->type = F;
		}
		else if (*s == 'G')
		{
			temprule->str[0] = 'G';
			temprule->str[1] = '\0';
			temprule->len = 1;
			temprule->type = G;
		}
		else if (*s == '+')
		{
			temprule->str[0] = '+';
			temprule->str[1] = '\0';
			temprule->len = 1;
			temprule->type = INC;
		}
		else if (*s == '-')
		{
			temprule->str[0] = '-';
			temprule->str[1] = '\0';
			temprule->len = 1;
			temprule->type = DEC;
		}
		else if (*s == '!')
		{
			temprule->str[0] = '!';
			temprule->str[1] = '\0';
			temprule->len = 1;
			temprule->type = REV;
		}
		else if (*s == '\\')
		{
			temprule->str[0] = '\\';
			temp_num = extract_number(s+1, temprule->str+1, 7);
			s += temp_num;
			temprule->len = temp_num + 1;
			temprule->type = INCX;
		}
		else if (*s == '/')
		{
			temprule->str[0] = '/';
			temp_num = extract_number(s+1, temprule->str+1, 7);
			s += temp_num;
			temprule->len = temp_num + 1;
			temprule->type = DECX;
		}
		else if (*s == '@')
		{
			temprule->str[0] = '@';
			temp_num = extract_number(s+1, temprule->str+1, 7);
			s += temp_num;
			temprule->len = temp_num + 1;
			temprule->type = SCALE;
		}
		else if (*s == 'D')
		{
			temprule->str[0] = 'D';
			temprule->str[1] = '\0';
			temprule->len = 1;
			temprule->type = D;
		}
		else if (*s == 'M')
		{
			temprule->str[0] = 'M';
			temprule->str[1] = '\0';
			temprule->len = 1;
			temprule->type = MM;
		}
		else if (*s == '[')
		{
			temprule->str[0] = '[';
			temprule->str[1] = '\0';
			temprule->len = 1;
			temprule->type = PUSH;
		}
		else if (*s == ']')
		{
			temprule->str[0] = ']';
			temprule->str[1] = '\0';
			temprule->len = 1;
			temprule->type = POP;
		}
		else if ((*s >= 'A') && (*s <= 'Z'))
		{
			temprule->str[0] = *s;
			temprule->str[1] = '\0';
			temprule->len = 1;
			temprule->type = RULE;
		}
		else
		{
			temprule--;
			count--;
		}
		s++;
		temprule++;
		count++;
	}

	/* Now we know the number of 'units' in this rule. */
	*rulelength = count;
	ret_rule = (ruleobj *)malloc(sizeof(ruleobj)*count);
	int i;
	for (i = 0; i < count; i++)
	{
		mystrcat(ret_rule[i].str, rule[i].str);
		ret_rule[i].len = rule[i].len;
		ret_rule[i].type = rule[i].type;
	}
	free(rule);
	return ret_rule;
}



void print_ls(const lsystem *ls)
{
	if (ls->computed_f)
		printf("Fitness: %f\n", ls->f);
	else
		printf("Fitness not computed\n");


	printf("Angle: %d\n", ls->angle);

	char *str = rule2string(ls->rule[0], ls->rulelength[0]);
	printf("Axiom: %s\n", str);
	free(str);

	int i;
	for (i = 1; i < ls->numrules; i++)
	{
		str = rule2string(ls->rule[i], ls->rulelength[i]);
		printf("%c -> %s\n", ls->startvar[i-1], str);
		free(str);
	}
}


void initialise_prng(unsigned long seed)
{
	init_genrand(&rng, seed);
}


lsystem *create_ls(const int num_rules, const int rule_length)
{
	lsystem *ls;
	ls = (lsystem *)malloc(sizeof(lsystem));
	/* The extra rule is the axiom. */
	ls->numrules = num_rules + 1;
	ls->angle = 3;

	ls->rule = (ruleobj **)malloc(sizeof(ruleobj *) * ls->numrules);
	ls->startvar = (char *)malloc(sizeof(char) * (ls->numrules - 1));

	int i;
	ls->rulelength = (int *)malloc(sizeof(int) * ls->numrules);
	for (i = 0; i < ls->numrules; i++)
	{
		ls->rule[i] = (ruleobj *)malloc(sizeof(ruleobj) * rule_length);
		ls->rulelength[i] = rule_length;
	}

	ls->computed_f = 0;

	return ls;
}



void delete_ls(lsystem *ls)
{
	int i, j;
	for (i = 0; i < ls->numrules; i++)
		free(ls->rule[i]);
	free(ls->rule);
	free(ls->rulelength);
	free(ls->startvar);
	free(ls);
}



void runningtime(char *filename, double *runtime, int *numbbnodes) {
	/* Create and open a pipe to the concorde program. */
	FILE *concorde_pipe;
	char *command;
	asprintf(&command, "concorde -s 0 %s", filename);
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

	*runtime = rtime;
	*numbbnodes = bbnodes;
}



/**
 *	\brief Creates NUM_ORDER_TEST TSPlib files representing the specified
 *	lsystem. The files are created in the current directory. The names of each file
 *	are returned in the array \c filenames. The size of each instance is stored
 *	in the array <c>ls->instancesize</c>.
 *
 *	You \e must delete the individual strings and the array itself when finished
 *	with them.
 */
void createTSPFiles(char ***filenames, lsystem *ls)
{
	char **fnames = (char **)malloc(sizeof(char *)*NUM_ORDER_TEST);

	int i, numnodes;
	printf("\nInstance Sizes: ");
	for (i = 0; i < NUM_ORDER_TEST; i++)
	{
		asprintf(&fnames[i], "plot_%d", i+1);
		savetofile(ls, "temp.l", "temp", NULL);

		/* Compute the x-y coordinates of the instance represented by this L-System. */
		numnodes = Lsystem(i + 1,"temp.l", "temp", "plots");
		ls->instancesize[i] = numnodes;
		printf("%d\t", numnodes);

		/* Now create a valid TSPlib file. */
		FILE *outfile_stream = fopen(fnames[i], "w");

		/* Check that the file has sucessfully been opened */
		if (outfile_stream == NULL)
		{
			printf("ERROR: File '%s' could not be opened\n", fnames[i]);
			return;
		}

		/* Build the header file */
		fprintf(outfile_stream, "NAME: %s\n", fnames[i]);
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
		remove("temp.l");

		fprintf(outfile_stream, "EOF:\n");

		/* Close the file */
		int close_error_plot = fclose(outfile_stream);
		if (close_error_plot)
		{
			printf("ERROR: File '%s' could not be closed\n", fnames[i]);
		}
	}

	printf("\n");

	(*filenames) = fnames;
}


/**
 *  \brief Computes the \e fitness of the specified L-System.
 *
 *	Note: In this case, the higher the fitness value, the better. This is
 *	because the fitness is proportional to the time taken for CONCORDE to solve
 *	the instance described by the L-System.
 *
 *	\param ls The iterated function system whose \e fitness will be calculated.
 *	\return A double value indicating how good the L-System is.
 */
double fitness(lsystem *ls)
{
	if (ls->computed_f)
	{
		return ls->f;
	}
	ls->f = 0;

	/* Create the TSPlib files. */
	char **filenames;
	createTSPFiles(&filenames, ls);

	print_ls(ls);

	int i, j, bb;
	int size = NUM_ORDER_TEST;
	data_point points[size];
	double rt, n, mean, S, delta;
	for (i = 0; i < NUM_ORDER_TEST; i++)
	{
		points[i].x = ls->instancesize[i];
		ls->avgbbnodes[i] = 0;
		printf("size: %.0f\n", points[i].x);


		points[i].y = 0;
		/* Run each instance twice and take the average.
		   Instances with large sizes take too long to solve and I am more
		   interested in the smaller sized instances anyway.               */
		if (points[i].x > 1500)
		{
			points[i].y = 0;
			printf("Size too large (> 1500 cities)\n");
		}
		else
		{
			n = mean = S = 0;
			for (j = 0; j < NUM_TSP_ITER; j++)
			{
				runningtime(filenames[i], &rt, &bb);
				ls->avgbbnodes[i] += (double)bb;
				printf("RT%2d: %0.4f\tBB: %d\n", j+1, rt, bb);

				/* This calculation is used to compute the standard deviation. */
				n++;
				delta = rt - mean;
				mean += delta/n;
				S += delta*(rt - mean); /* NOTE: This expression uses the new value of mean. */
			}
			points[i].y = mean;
			ls->runningtimes[i] = points[i].y;
			ls->avgbbnodes[i] /= NUM_TSP_ITER;
			printf("\nAVG: %0.4f(%0.4f)\n", ls->runningtimes[i], ls->avgbbnodes[i]);

			S = S/(n - 1);
			ls->sd[i] = fastsqrt(S);
			printf("SD:  %0.4f\n", ls->sd[i]);
			printf("68%% lies within:  (%7.4f, %7.4f)\n", ls->runningtimes[i] - ls->sd[i], ls->runningtimes[i] + ls->sd[i]);
			printf("95%% lies within:  (%7.4f, %7.4f)\n", ls->runningtimes[i] - 2*(ls->sd[i]), ls->runningtimes[i] + 2*(ls->sd[i]));
		}

		printf("\n");
		free(filenames[i]);
	}
	free(filenames);
	system("rm -f Oplot* plot*");


	for (i = 0; i < NUM_ORDER_TEST; i++)
	{
		if (ls->instancesize[i] == ls->instancesize[i-1])
		{
			ls->computed_f = 1;
			ls->f = 0;
			printf("Some instance sizes are the same!!\n");
			printf("fitness: %f\n", ls->f);
			return ls->f;
		}
	}

	i = 0;
	while ((points[i].x < 100) || (points[i].y == 0)) i++;
	int offset = i-1;

	/* We might not like too many (x,0) points, but we need to have at least
	   three points to compute the upper bound. */
	if (size - offset < 3) offset = size - 3;

	/* Compute the upper bound of the running times of the plots of different sizes. */
	double func[5];
	find_upper_bound(func, points + offset, size - offset);

	/* Compute the sum of errors. */
	double se = sum_error(func, points + offset, size - offset);

	/* We want to maximise 'func[0]' and minimise 'se'. */
	/* TODO: Incorporate all the ls->numbbnodes[i] into the fitness function. */
	ls->f = (func[0]*func[0])/(se+1);
	ls->computed_f = 1;

	printf("UB y =  %0.3f + %0.3f((x - %0.3f)/%0.3f)^(%0.6f)\n", func[2], func[4], func[1], func[3], func[0]);
	printf("SE: %f\n", se);
	printf("fitness: %f\n", ls->f);

	return ls->f;
}



int compare_ls(lsystem **ls1, lsystem **ls2)
{
	double ls1_value = fitness(*ls1);
	double ls2_value = fitness(*ls2);

	if (ls1_value == ls2_value)
		return 0;
	if (ls1_value > ls2_value)
		return -1;
	else
	    return 1;
}



/**
 *	Sets a random value for the given ruleobj object.
 *
 *	\param robj The ruleobj whose values will be set.
 *	\param ls   The L-System the given ruleobj belongs to.
 */
void randomObj(ruleobj *robj, lsystem *ls)
{
	int randangle, randrule;
	double randnum;
	int objtype = (int)(genrand_int32(&rng)%9);

	switch (objtype)
	{
		case 0:
			robj->str[0] = 'F';
			robj->str[1] = '\0';
			robj->len = 1;
			robj->type = F;
			break;

		case 1:
			robj->str[0] = 'G';
			robj->str[1] = '\0';
			robj->len = 1;
			robj->type = G;
			break;

		case 2:
			robj->str[0] = '+';
			robj->str[1] = '\0';
			robj->len = 1;
			robj->type = INC;
			break;

		case 3:
			robj->str[0] = '-';
			robj->str[1] = '\0';
			robj->len = 1;
			robj->type = DEC;
			break;

		case 4:
			robj->str[0] = '!';
			robj->str[1] = '\0';
			robj->len = 1;
			robj->type = REV;
			break;

		case 5:
			/* Generate a random number within the range (1,359) */
			randangle = (int)(genrand_int32(&rng)%359) + 1;
			sprintf(robj->str, "\\%03d", randangle);
			robj->len = 4;
			robj->type = INCX;
			break;

		case 6:
			/* Generate a random number within the range (1,359) */
			randangle = (int)(genrand_int32(&rng)%359ul) + 1;
			sprintf(robj->str, "/%03d", randangle);
			robj->len = 4;
			robj->type = DECX;
			break;

		case 7:
			/* Generate a random number within the range (0,10) */
			randnum = genrand_real3(&rng);
			/* Make sure the number doesn't get rounded down to 0.00 or up to 10.00 */
			if (randnum < 0.005) randnum = 0.01;
			if (randnum > 9.994) randnum = 9.99;
			sprintf(robj->str, "@%04.2f", randnum);
			robj->len = 5;
			robj->type = SCALE;
			break;

		case 8:
			randrule = (int)(genrand_int32(&rng)%(ls->numrules-1));
			robj->str[0] = ls->startvar[randrule];
			robj->str[1] = '\0';
			robj->len = 1;
			robj->type = RULE;
			break;
	}
}


void random_angle(lsystem *ls)
{
	ls->angle = (int)genrand_int32(&rng)%(48UL) + 3;
}


void random_rule(lsystem *ls, int rule)
{
	int i;
	for (i = 0; i < ls->rulelength[rule]; i++)
	{
		randomObj(&(ls->rule[rule][i]), ls);
	}

	/* We'll make it a one in NUM_OBJ_TYPES chance of placing a stack. */
	int rolldie = (int)(genrand_int32(&rng)%(long)NUM_OBJ_TYPES);
	if (!rolldie)
	{
		unsigned long pos1 = genrand_int32(&rng)%(ls->rulelength[rule]);
		unsigned long pos2 = genrand_int32(&rng)%(ls->rulelength[rule]);

		unsigned long start = (pos1 < pos2 ? pos1 : pos2);
		unsigned long end   = (pos1 < pos2 ? pos2 : pos1);
		/* Can't be the same or next to each other */
		if ((end - start) > 1)
		{
			ls->rule[rule][start].str[0] = '[';
			ls->rule[rule][start].str[1] = '\0';
			ls->rule[rule][start].len = 1;
			ls->rule[rule][start].type = PUSH;

			ls->rule[rule][end].str[0] = ']';
			ls->rule[rule][end].str[1] = '\0';
			ls->rule[rule][end].len = 1;
			ls->rule[rule][end].type = POP;
		}
	}

	/* We should avoid the situation where this rule has no A,B,C ... etc
	   otherwise the L-System may not grow after each iteration. */
	int randpos = (int)(genrand_int32(&rng)%ls->rulelength[rule]);
	if ((ls->rule[rule][randpos].type == PUSH) || (ls->rule[rule][randpos].type == POP))
		if (randpos == 0)
			randpos++;
		else
			randpos--;

	int randrule = (int)(genrand_int32(&rng)%(ls->numrules-1));
	ls->rule[rule][randpos].str[0] = ls->startvar[randrule];
	ls->rule[rule][randpos].str[1] = '\0';
	ls->rule[rule][randpos].len = 1;
	ls->rule[rule][randpos].type = RULE;
}



void crossover_ls(const lsystem *p1, const lsystem *p2, lsystem *c1, lsystem *c2)
{
	/* Iterate over the rules. */
	int i, j, pos1, pos2, length1, length2, foundStackStart, foundStackEnd;
	for (i = 0; i < c1->numrules; i++)
	{
		/* Randomly select a crossover point. */
		pos1 = (int)(genrand_int32(&rng)%(p1->rulelength[i]));
		pos2 = (int)(genrand_int32(&rng)%(p2->rulelength[i]));

		/* We cannot split a stack section. */
		j = pos1;
		foundStackStart = 0;
		foundStackEnd = 0;
		while ((j > 0) && (!foundStackEnd) && (!foundStackStart))
		{
			j--;
			if (p1->rule[i][j].type == POP)
				foundStackEnd = 1;
			if (p1->rule[i][j].type == PUSH)
			{
				foundStackStart = 1;
			}
		}

		if (foundStackStart)
		{
			/* Find where the stack ends. */
			j = pos1-1;
			while ((j < p1->rulelength[i]) && (!foundStackEnd))
			{
				j++;
				if (p1->rule[i][j].type == POP)
					foundStackEnd = 1;
			}
			pos1 = j+1;
		}


		/* Again for the second child/rule, we cannot split a stack section. */
		j = pos2;
		foundStackStart = 0;
		foundStackEnd = 0;
		while ((j > 0) && (!foundStackEnd) && (!foundStackStart))
		{
			j--;
			if (p2->rule[i][j].type == POP)
				foundStackEnd = 1;
			if (p2->rule[i][j].type == PUSH)
				foundStackStart = 1;
		}

		if (foundStackStart)
		{
			/* Find where the stack ends. */
			j = pos2-1;
			while ((j < p2->rulelength[i]) && (!foundStackEnd))
			{
				j++;
				if (p2->rule[i][j].type == POP)
					foundStackEnd = 1;
			}
			pos2 = j+1;
		}

		/* Based on the crosover points, we now know what will be the lengths of
		   the two new rules that will be created.                               */
		length1 = pos1 + (p2->rulelength[i] - pos2);
		length2 = pos2 + (p1->rulelength[i] - pos1);

		/* Remove the old rules and replace them with new ones. */
		c1->rulelength[i] = length1;
		free(c1->rule[i]);
		c1->rule[i] = (ruleobj *)malloc(sizeof(ruleobj) * length1);

		c2->rulelength[i] = length2;
		free(c2->rule[i]);
		c2->rule[i] = (ruleobj *)malloc(sizeof(ruleobj) * length2);

		/* Now create the new rules. */
		memcpy(c1->rule[i], p1->rule[i], pos1*sizeof(ruleobj));
		memcpy(c1->rule[i] + pos1, p2->rule[i] + pos2, (length1 - pos1)*sizeof(ruleobj));

		memcpy(c2->rule[i], p2->rule[i], pos2*sizeof(ruleobj));
		memcpy(c2->rule[i] + pos2, p1->rule[i] + pos1, (length2 - pos2)*sizeof(ruleobj));
	}

	/* Inherit the angle values.
	   TODO: Change this part to something better. The values aren't exactly evolving. */
	c1->angle = p1->angle;
	c2->angle = p2->angle;

	c1->computed_f = 0;
	c2->computed_f = 0;
}




void savetofile(lsystem *ls, char *filename, char *lsname, char *comments)
{
	/* Create a new ls file */
	FILE *outfile_stream = fopen(filename, "a");

	/* Store the fitness value of this iterated function system. */
	if (ls->computed_f)
		fprintf(outfile_stream, "; Fitness: %f\n", ls->f);
	else
		fprintf(outfile_stream, "; Fitness not computed\n");

	/* Store (in a commented line) the size of each plot size tested. */
	int i;
	fprintf(outfile_stream, ";\n; Instance Sizes:\n;");
	for (i = 0; i < NUM_ORDER_TEST; i++)
		fprintf(outfile_stream, " %d", ls->instancesize[i]);
	fprintf(outfile_stream, "\n");

	/* Store (in a commented line) the times taken for each plot size tested. */
	fprintf(outfile_stream, ";\n; Running Times:\n;");
	for (i = 0; i < NUM_ORDER_TEST; i++)
		fprintf(outfile_stream, " %f", ls->runningtimes[i]);
	fprintf(outfile_stream, "\n");

	/* Store (in a commented line) the upper bound on the running times. */
	fprintf(outfile_stream, ";\n; Running Times bounded above by:\n;");
	data_point points[NUM_ORDER_TEST];
	for (i = 0; i < NUM_ORDER_TEST; i++)
	{
		points[i].x = ls->instancesize[i];
		points[i].y = ls->runningtimes[i];
	}
	double func[5];
	find_upper_bound(func, points, NUM_ORDER_TEST);
	fprintf(outfile_stream, " t(n) = %f((n - %f)/%f)^(%f) + %f\n", func[4], func[1], func[3], func[0], func[2]);


	/* Store any additional comments */
	char *str, *newlinepos, *prevlinepos;
	int length;
	if (comments != NULL)
	{
		char *comments2;
		comments2 = (char *)malloc(sizeof(char)*strlen(comments));
		strcpy(comments2, comments);

		/* Get the first line. */
		newlinepos = strchr(comments2, '\n');
		if (newlinepos == NULL)
		{
			/* Get the remainder of the line. */
			newlinepos = strchr(comments2, '\0');
		}
		prevlinepos = comments2;

		/* Use the positions of where our line starts and ends to find the length
		   of the string.                                                         */
		length = newlinepos - prevlinepos;

		do
		{
			/* Get a copy of the substring (i.e. the next line). */
			str = (char *)malloc(sizeof(char)*length + 1);
			strncpy(str, prevlinepos, length);
			str[length] = '\0';

			/* Print the next line to the file. */
			fprintf(outfile_stream, "; %s\n", str);
			free(str);

			/* Get the 'next line'. That is, start from the position after the '\n'.*/
			prevlinepos = newlinepos + 1;
			newlinepos = strchr(prevlinepos, '\n');

			if (newlinepos == NULL)
			{
				/* Get the remainder of the line. */
				newlinepos = strchr(comments2, '\0');
			}

			/* Use the positions where our line starts and ends to find the length
			   of the string.                                                      */
			length = newlinepos - prevlinepos;
		} while (length >= 0);

		free(comments2);
	}

	/* Store the L-System to file. */
	fprintf(outfile_stream, "%s {\n", lsname);
	fprintf(outfile_stream, "Angle %d\n", ls->angle);

	char *tempstr = rule2string(ls->rule[0], ls->rulelength[0]);
	fprintf(outfile_stream, "Axiom %s\n", tempstr);
	free(tempstr);

	for (i = 1; i < ls->numrules; i++)
	{
		tempstr = rule2string(ls->rule[i], ls->rulelength[i]);
		fprintf(outfile_stream, "%c=%s\n", ls->startvar[i-1], tempstr);
		free(tempstr);
	}
	fprintf(outfile_stream, "}\n\n\n");

	/* Close the file */
	int close_error_func = fclose(outfile_stream);
	if (close_error_func)
	{
		printf("ERROR: File '%s' could not be closed\n", filename);
	}
}


#define LINE_LENGTH 160

/**
 *	\brief Returns the number of L-Systems in the given file.
 *
 *	\param fp A file pointer to an L-System file.
 *	\return   The number of L-Systems in the file.
 */
int numLSystemsInFile(FILE *fp)
{
	int count = 0;
	char line[LINE_LENGTH], *temp, *delims = " \t\n";

	rewind(fp);
	while (fgets(line, LINE_LENGTH, fp) != NULL)
	{
		if (line[0] != ';') /* Ignore comments. */
		{
			/* See if we've reached the end. */
			if (strchr(line, '}')) count++;
		}
	}
	rewind(fp);
	return count;
}


/**
 *	\brief Reads the L-System located immediately after the position in the
 *	file pointed to by the given file pointer.
 *
 *	\param fp A file pointer to an L-System file pointing at an L-System.
 *	\param ls The lsystem object in which to store the L-System information.
 */
void readLSystem(FILE *fp, lsystem *ls)
{
	int rulecount = 0, m, n, i=1;
	char line[LINE_LENGTH], *temp, *delims = " :;=\t\n", rule[LINE_LENGTH], c;

	fpos_t file_loc;
	fgetpos(fp, &file_loc);

	while (fgets(line, LINE_LENGTH, fp) != NULL)
	{
		if (line[0] != ';') /* Ignore comments. */
		{
			/* See if this line has a rule. */
			if (temp = strchr(line, '='))
			{
				rulecount++;
			}
			/* See if we've reached the end. */
			if (strchr(line, '}')) break;
		}
	}
	/* The extra rule is the axiom. */
	ls->numrules = rulecount + 1;
	ls->rule = (ruleobj **)malloc(sizeof(ruleobj *) * ls->numrules);
	ls->startvar = (char *)malloc(sizeof(char) * (ls->numrules - 1));
	ls->rulelength = (int *)malloc(sizeof(int) * ls->numrules);

	fsetpos(fp, &file_loc);
	while (fgets(line, LINE_LENGTH, fp) != NULL)
	{
		/* We Ignore comments by replacing the first ';' with NULL.
		   This way the string terminates before the comments start. */
		temp = strchr(line, ';');
		if (temp != NULL) *temp = '\0';

		strupr(line);
		temp = strtok(line, delims);
		if (temp != NULL)
		{
			if (!strcmp(temp, "ANGLE"))
			{
				temp = strtok(NULL, delims);
				ls->angle = (int)strtol(temp, NULL, 10);
			}
			else if (!strcmp(temp, "AXIOM"))
			{
				temp = strtok(NULL, delims);
				ls->rule[0] = string2rule(temp, &n);
				ls->rulelength[0] = n;
			}
			else
			{
				/* We're analysing a rule. */
				c = temp[0];
				temp = strtok(NULL, delims);
				if (temp != NULL)
				{
					ls->startvar[i-1] = c;
					ls->rule[i] = string2rule(temp, &n);
					ls->rulelength[i] = n;
					i++;
				}
			}
		}
		/* See if we've reached the end. */
		if (strchr(line, '}'))
		{
			ls->computed_f = 0;
			return;
		}
	}
}


void standardise_rules(lsystem **pop, int popsize)
{
	/* Rename the rules to "A,B,C,..." so that the crossover and mutation functions
	   will work. We'll use capital letters (ASCII values 65 - 90), but to avoid
	   the issue where these letters may already be in use, we'll initially use the
	   tempoary values 0 - 26. */
	int i, j, p;
	char chr[26], rep[26], num;

	for (i = 0; i < 26; i++)
		chr[i] = i + 65;


	for (p = 0; p < popsize; p++)
	{
		for (i = 0; i < 26; i++)
			rep[i] = 42;

		num = 0;
		for (i = 0; i < pop[p]->numrules; i++)
		{
			if ((pop[p]->startvar[i] != 'D') && (pop[p]->startvar[i] != 'F') && (pop[p]->startvar[i] != 'G') && (pop[p]->startvar[i] != 'M'))
			{
				rep[pop[p]->startvar[i] - 65] = num;
				/* Don't forget to do the replacement in the startvar array. */
				pop[p]->startvar[i] = num + 65;
				num++;
				if ((num == 3) || (num == 12))
				{
					/* Note: because of the next if statement, num won't be 6 (G). */
					num++;
				}
				if (num == 5)
					num = num + 2;
			}
			else
			{
				rep[pop[p]->startvar[i] - 65] = pop[p]->startvar[i];
			}
		}

		/* Do the first substitution (original letters -> temporary values). */
		/* Iterate over the rules. */
		for (i = 0; i < pop[p]->numrules; i++)
		{
			/* Iterate over the ruleobj's. */
			for (j = 0; j < pop[p]->rulelength[i]; j++)
			{
				if (pop[p]->rule[i][j].type == RULE)
				{
					pop[p]->rule[i][j].str[0] = rep[pop[p]->rule[i][j].str[0] - 65];
				}
			}
		}

		/* Now do the second substitution (original letters -> temporary values). */
		/* Iterate over the rules. */
		for (i = 0; i < pop[p]->numrules; i++)
		{
			/* Iterate over the ruleobj's. */
			for (j = 0; j < pop[p]->rulelength[i]; j++)
			{
				if (pop[p]->rule[i][j].type == RULE)
				{
					pop[p]->rule[i][j].str[0] += 65;
				}
			}
		}
	}
}


int readfile(FILE *fp, lsystem ***pop, char ***names)
{
	int error, n, i;
	char line[LINE_LENGTH], *temp, **_names;

	if (!fp)
	{
		printf("ERROR: Couldn't open file in function 'readfile(FILE *fp, lsystem ***pop, char ***names)\n");
		exit(1);
	}

	n = numLSystemsInFile(fp);
	if (names != NULL) _names = (char **)malloc(sizeof(char *)*n);
	*pop = (lsystem **)malloc(sizeof(lsystem *)*n);
	for (i = 0; i < n; i++)
		(*pop)[i] = (lsystem *)malloc(sizeof(lsystem));

	i = 0;
	while (fgets(line, LINE_LENGTH, fp) != NULL)
	{

		if (line[0] != ';') /* Ignore comments. */
		{
			/* Search for newline character. */
			temp = strchr(line,'\n');
			/* Overwrite trailing newline.   */
			if (temp != NULL)
				*temp = '\0';
			else
				line[LINE_LENGTH - 1] = '\0';

			/* We look for the L-System by finding a '{' */
			temp = strchr(line, '{');
			if (temp)
			{
				temp = strtok(line, " ");
				if (names != NULL)
				{
					_names[i] = (char *)malloc(strlen(temp));
					strcpy(_names[i], temp);
				}
				readLSystem(fp, (*pop)[i]);
				i++;
			}
		}
	}

	standardise_rules(*pop, n);

	/*
	for (i = 0; i < n; i++)
		print_ls((*pop)[i]);
	exit(0);
	*/

	if (names != NULL) *names = _names;
	return n;
}
