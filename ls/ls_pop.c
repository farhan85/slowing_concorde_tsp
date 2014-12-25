/**
 *	\file
 *	\brief Implements the methods defined in the file ls_pop.h.
 *
 *	\author Farhan Ahammed (faha3615@mail.usyd.edu.au)
 */

#include <stdio.h>
#include <string.h>

#include "ls_pop.h"
#include "../sortalg/mergesort_ls.h"
#include "../mt19937ar/t_mt19937ar.h"


mt_prng prng;


void printall(ls_population *pop)
{
	int i;
	for (i = 0; i < pop->pop_size; i++)
	{
		printf("L-System #%d\t", i+1);
		print_ls(pop->ls_array[i]);
	}
}


void printbest(ls_population *pop)
{
	print_ls(pop->ls_array[0]);
}


double getfitness(ls_population *pop, int i)
{
	if (i >= 0 && i < pop->pop_size)
	{
		return fitness(pop->ls_array[i]);
	}
	else
		return 0;
}


void createPopulation(ls_population **pop, const int numparents, const int num_rules, const int rule_size)
{
	ls_population *temp_pop = (ls_population *)malloc(sizeof(ls_population));

	lsystem **temp_ls;
	/* The code is simpler if there is always an even number of parents. */
	if (numparents%2 == 1)
	{
		temp_pop->pop_size = numparents + numparents + 2;
		temp_pop->num_parents = numparents + 1;
	}
	else
	{
		temp_pop->pop_size = numparents + numparents;
		temp_pop->num_parents = numparents;
	}
	temp_ls = (lsystem **)malloc(sizeof(lsystem *)*temp_pop->pop_size);

	int i;
	for (i = 0; i < temp_pop->pop_size ; i++)
	{
		temp_ls[i] = create_ls(num_rules, rule_size);
	}

	temp_pop->numrules = num_rules + 1;
	temp_pop->ls_array = temp_ls;

	*pop = temp_pop;
}


void deletePopulation(ls_population *pop)
{
	int i;
	for (i = 0; i < pop->pop_size; i++)
	{
		delete_ls(pop->ls_array[i]);
	}

	free(pop->ls_array);
	free(pop);
}


/**
 *	\brief Returns -1 if ls1 > ls2, 0 if they are equal, 1 otherwise.
 *
 *	The lsystem structures are compared by their fitness. This
 *	method will return
 *	- \c -1 if <i>f(ls1) > f(ls2)</i>,
 *	- \c 0  if <i>f(ls1) = f(ls2)</i> and
 *	- \c 1  if <i>f(ls1) < f(ls2)</i>.
 *
 *	Using this function in a sorting algorithm will result in the collection
 *	of lsystem structures sorted from largest fitness to least.
 *
 *	\param ls1 An lsystem structure.
 *	\param ls2 Another lsystem structure.
 *	\return \c -1 if ls1 > ls2, \c 0 if they are equal, \c 1 otherwise.
 */
int compare(lsystem **ls1, lsystem **ls2)
{
	return compare_ls(ls1, ls2);
}


void sortpopulation(ls_population *pop)
{
	mergesort_ls(pop->ls_array, pop->pop_size, &compare);
}


void assignRandomValues(ls_population *pop)
{
	initialise_prng(time(NULL));
	init_genrand(&prng, time(NULL));

	/* Note: There are (numrules - 1) rules plus one axiom.
	   For each rule, we have to assign it a letter. */
	char *rulename, currentsymbol = 'A';
	int i;

	rulename = (char *)malloc((pop->numrules - 1));

	/* Now assign the letters (names of each rule). */
	i = 0;
	while (i < pop->numrules-1)
	{
		if ((currentsymbol != 'D') && (currentsymbol != 'F') && (currentsymbol != 'G') && (currentsymbol != 'M'))
		{
			rulename[i] = currentsymbol;
			i++;
		}
		currentsymbol++;
	}

	lsystem **ls = pop->ls_array;
	int j;

	/* Iterate through each L-System. */
	for (i = 0; i < pop->pop_size; i++)
	{
		/* Make sure the L-System knows the names of each rule. */
		for (j = 0; j < ls[i]->numrules; j++)
			ls[i]->startvar[j] = rulename[j];

		/* Iterate through each rule. */
		for (j = 0; j < ls[i]->numrules; j++)
			random_rule(ls[i], j);

		random_angle(ls[i]);
	}
	free(rulename);
}


/**
 *	\brief Returns two random but unique parents (\e not according to the uniform
 *	distribution).
 *
 *	Assumption: The population is already sorted.
 *
 *	The first parent returned is selected at random according to the following
 *	probability distribution:
 *
 *	<center>
 *	<table border="0" cellpadding="6">
 *		<tr><th>IFS</th><th>P(i)</th></tr>
 *		<tr><td><code>pop->ifs_array[0]</code></td><td>50%</td></tr>
 *		<tr><td><code>pop->ifs_array[1]</code></td><td>25%</td></tr>
 *		<tr height="1px"><td colspan=2 align="center">.</td></tr>
 *		<tr height="1px"><td colspan=2 align="center">.</td></tr>
 *		<tr height="1px"><td colspan=2 align="center">.</td></tr>
 *		<tr><td><code>pop->ifs_array[i]</code></td><td>P(i-1)/2</td></tr>
 *	</table>
 *	</center>
 *
 *	The second parent returned is selected according to a similar probability
 *	distribution but the possible choices do not include the first (selected) parent.
 *
 *	\param pop The L-System population where the parents are chosen from (and children are placed).
 *	\param p1  A pointer to the first chosen parent will be returned in this parameter.
 *	\param p2  A pointer to the second chosen parent will be returned in this parameter.
 */
void chooseUniqueParents(ls_population *pop, lsystem **p1, lsystem **p2)
{
	double *weight = (double *)malloc(sizeof(double)*(pop->num_parents));
	int *pos = (int *)malloc(sizeof(int)*(pop->num_parents));

	double temp = 100;
	int i;
	for (i = 0; i < pop->num_parents - 1; i++)
	{
		temp = temp/2;
		weight[i] = temp;
		pos[i] = i;
	}
	weight[pop->num_parents - 1] = 0;
	pos[pop->num_parents - 1] = pop->num_parents - 1;

	/* Select the first parent. */
	double n = 100*genrand_real1(&prng);
	int foundOne = 0;
	i = 0;
	while ((i < pop->num_parents) && (!foundOne))
	{
		if (n > weight[i])
		{
			(*p1) = pop->ls_array[pos[i]];
			foundOne = 1;

			/* Now readjust the 'probability distribution'. We do this by
			   shifting the remaining 'pos' numbers up by one.            */
			int j;
			for (j = i; j < pop->num_parents - 1; j++)
				pos[j] = pos[j + 1];

			if (i == (pop->num_parents - 1))
				pos[pop->num_parents - 1] = pos[pop->num_parents - 2];
		}
		i++;
	}

	/* Select the second parent. */
	n = 100*genrand_real1(&prng);
	foundOne = 0;
	i = 0;
	while (i < pop->num_parents)
	{
		if (n > weight[i])
		{
			(*p2) = pop->ls_array[pos[i]];

			free(weight);
			free(pos);
			return;
		}
		i++;
	}

	/* Note: We shouldn't really get here. But in case we do, just return the first two parents. */
	free(weight);
	free(pos);
	(*p1) = pop->ls_array[0];
	(*p2) = pop->ls_array[1];
	fprintf(stderr, "ERROR in method 'chooseUniqueParents(ls_population *, lsystem **, lsystem **)' ");
	fprintf(stderr, "when selecting the second parent.\n");
	fprintf(stderr, "We've reached a part of the code we shouldn't have\n");
}


void generateChildren(ls_population *pop)
{
	/* Note: The population should be sorted FIRST. */

	/* The first position where the child is located in in position 'pop->num_parents' */
	int i;
	lsystem *p1, *p2;
	for (i = pop->num_parents; i < pop->pop_size - 1; i = i + 2)
	{
		chooseUniqueParents(pop, &p1, &p2);
		crossover_ls(p1, p2, pop->ls_array[i], pop->ls_array[i+1]);
	}
}


void mutateChildren(ls_population *pop)
{
	/* Select a random child. */
	int i = (int)(pop->num_parents + genrand_int32(&prng)%(pop->pop_size - pop->num_parents));
	lsystem *randchild = pop->ls_array[i];

	/*  Now select a random rule. */
	int randrule = (int)(genrand_int32(&prng)%(pop->numrules));

	random_rule(randchild, randrule);

	random_angle(randchild);
}


void savelstofile(ls_population *pop, int i, char *filename, char *lsname, char *comments)
{
	savetofile(pop->ls_array[i], filename, lsname, comments);
}

