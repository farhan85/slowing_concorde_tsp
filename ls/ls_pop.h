/**
 *	\file
 *	\brief Defines a population of lsystem structures and some
 *	methods needed to work with them.
 *
 *	\author Farhan Ahammed (faha3615@mail.usyd.edu.au)
 */

#ifndef LS_POP_H
#define LS_POP_H

#include <stdio.h>
#include <stdlib.h>

#include "ls.h"



/**
 *	\brief A population of L-Systems.
 */
typedef struct
{
	/**
	 *	\brief The number of rules in every L-System in this population. It has
	 *	to be at least two since the first one is the \e axiom.
	 */
	int numrules;

	/**
	 *	\brief The number of parents in the population.
	 */
	int num_parents;

	/**
	 *	\brief The size of the population.
	 *
	 *	The size of the population is at most\n
	 *	<c>number_of_parents + number_of_parents</c> = <i>number of parents</i> + <i>number of children</i>.\n
	 *	This is because every pair of parents produces two children.
	 */
	int pop_size;

	/**
	 *	\brief The population itself.
	 */
	lsystem **ls_array;

} ls_population;


/**
 *	\brief Prints a formatted output of the population of lsystem
 *	structures on screen.
 *
 *	\param pop An ls population.
 */
void printall(ls_population *pop);

/**
 *	\brief Returns the fitness of the specified L-System in the population
 *	(which is represented as an array).
 *
 *	Note: The array is sorted first before obtaining the fitness value.
 *
 *	\param  pop An ls population.
 *	\param  i   The position in the array of the ls whose fitness is wanted.
 *	\return The fitness of the ls in position i.
 */
double getfitness(ls_population *pop, int i);


/**
 *	\brief Allocates memory for a new ls_population structure.
 *
 *	The structure is returned in the ls_population pointer given as a parameter.
 *
 *	\param pop        A pointer to an ls_population structure.
 *	\param numparents The number of parents in the population.
 *	\param num_rules  The number of rules the specified lsystem structure will hold.
 *	\param rule_size  The length or each rule (initially).
 */
void createPopulation(ls_population **pop, const int numparents, const int num_rules, const int rule_size);

/**
 *	\brief Deletes the specified ls_population structure from the heap.
 *
 *	\param pop The ls population to delete.
 */
void deletePopulation(ls_population *pop);

/**
 *	\brief Sorts the specified population from best individual to worst.
 *	\param pop The population, whose array is to be sorted.
 */
void sortpopulation(ls_population *pop);

/**
 *  \brief Initialises the population.
 *
 *	Goes through each coefficient structure and sets a random number to each coefficient.
 *
 *	\param pop The ls population to add random individuals to.
 */
void assignRandomValues(ls_population *pop);

/**
 *	\brief Generates children in the population.
 *
 *	NOTE: The population should be sorted \e before calling this function.
 *
 *	\param pop The ls population to generate children.
 */
void generateChildren(ls_population *pop);

/**
 *	\brief Mutates the children in the population.
 *
 *	Assumption: The children have already been generated.
 *
 *	\param pop The ls population whose children is to be mutated.
 *	\see generateChildren(ls_population *pop)
 */
void mutateChildren(ls_population *pop);

/**
 *	\brief Saves the specified L-System in the population (which is represented
 *	as an array) to file.
 *
 *	Note: The array is sorted first before saving to file.
 *
 *	\param pop       An ls population.
 *	\param i         The position in the array of the ls to save to file.
 *	\param filename  The name of the file to write to.
 *	\param lsname   The name to assign to the L-System.
 *	\param comments  Any additional comments to be added to the created files.
 */
void savelstofile(ls_population *pop, int i, char *filename, char *lsname, char *comments);

#endif
