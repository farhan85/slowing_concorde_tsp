/**
 *	\file
 *	\brief Defines an <i>L-System</i> structure and some methods needed to work with them.
 *
 *	\author Farhan Ahammed (faha3615@mail.usyd.edu.au)
 */

#ifndef LS_H
#define LS_H

#include <stdio.h>
#include <stdlib.h>


/**
 *	\brief The number of orders of the L-System we will create and test.
 */
#define NUM_ORDER_TEST 3



/* The types of of each 'object' that are used in a rule. */
#define F       1
#define G       2
#define INC     3
#define DEC     4
#define REV     5
#define PUSH    6
#define POP     7
#define INCX    8
#define DECX    9
#define SCALE  10
#define RULE   11
#define D      12  //TODO: Use the 'D' and 'M' in your evolutionary algorithm.
#define MM     13  /* Can't use 'M' because it's used in the Mersenne Twister PRNG code. */

#define NUM_OBJ_TYPES 11

/**
 *	\brief A single 'unit' in a rule.
 */
typedef struct {
	char str[8], len, type;
} ruleobj;




/**
 *	\brief An L-System.
 */
typedef struct
{
	/**
	 *	\brief The number of rules in this L-System. It has to be at least two since
	 *	the first one is the \e axiom.
	 */
	int numrules;

	/**
	 *	\brief The set of rules.
	 *
	 *	It is an array of rules (which each is an array of ruleobj pointers) of size
	 *	#numrules. The first rule is the axiom.
	 */
	ruleobj **rule;

	/**
	 *	\brief The name of each rule (ie. the start variable) - except the first
	 *	(which is the \e axiom).
	 */
	char *startvar;

	/**
	 *	\brief The length of each rule in this L-System.
	 */
	int *rulelength;

	/**
	 *	\brief The unit angle for this L-System. The angle is computed as
	 *	360/<c>angle</c> degrees.
	 */
	int angle;

	/**
	 *	\brief The size of each instance used for testing.
	 */
	int instancesize[NUM_ORDER_TEST];

	/**
	 *	\brief A boolean value indicating whether or not the \e fitness of this
	 *	L-System has been calculated.
	 */
	int computed_f;

	/**
	 *	\brief The average time it takes for CONCORDE to solve instances created
	 *	by this L-System of various sizes.
	 */
	double runningtimes[NUM_ORDER_TEST];

	/**
	 *	\brief The standard deviation of the set of running times.
	 */
	double sd[NUM_ORDER_TEST];

	/**
	 *	\brief The number of branch-and-bound nodes needed by CONCORDE to solve
	 *	instances created by this L-System of various sizes.
	 */
	double avgbbnodes[NUM_ORDER_TEST];

	/**
	 *	\brief The \e fitness of this L-System.
	 */
	double f;
} lsystem;


/**
 *	\brief Initialise the pseudo random number generator used by the lsystem objects.
 *
 *	\param seed The value of the initial seed to use.
 */
void initialise_prng(unsigned long seed);

/**
 *	\brief Creates a new lsystem structure with \c size rules.
 *
 *	\param num_rules The number of rules the specified lsystem structure will hold.
 *	\param rule_size The length or each rule (initially).
 *	\return The lsystem structure that was created.
 *	\sa delete_ls(lsystem *ls)
 */
lsystem *create_ls(const int num_rules, const int rule_size);

/**
 *	\brief Deletes the specified lsystem structure.
 *
 *	\param ls The lsystem structure to delete.
 *	\see create_ls(const int size)
 */
void delete_ls(lsystem *ls);

/**
 *	\brief Prints a formatted output of the specified L-System.
 *
 *	\param ls An lsystem.
 */
void print_ls(const lsystem *ls);

/**
 *	\brief Runs \e Concorde on the instance (stored in a file) and returns the
 *	time taken and the number of branch-and-bound nodes needed to solve it.
 *
 *	\param filename   The TSPlib file to make \e Concorde try to solve.
 *	\param runtime    The running time is stored in the double pointed to by
 *	                  this parameter.
 *	\param numbbnodes The number of branch-and-bound nodes needed by concorde
 *	                  is stored in the int pointed to by this parameter.
 */
void runningtime(char *filename, double *runtime, int *numbbnodes);

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
double fitness(lsystem *ls);

/**
 *	\brief Returns -1 if ls1 > ls2, 0 if they are equal, 1 otherwise.
 *
 *	The lsystem structure are compared by their fitness. This
 *	method will return
 *	- \c -1 if <i>f(c1) > f(c2)</i>,
 *	- \c 0  if <i>f(c1) = f(c2)</i> and
 *	- \c 1  if <i>f(c1) < f(c2)</i>.
 *
 *	Using this function in a sorting algorithm will result in the collection
 *	of lsystem structures sorted from largest fitness to least.
 *
 *	\param ls1 An lsystem structure.
 *	\param ls2 Another lsystem structure.
 *	\return \c -1 if ls1 > ls2, \c 0 if they are equal, \c 1 otherwise.
 */
int compare_ls(lsystem **ls1, lsystem **ls2);

/**
 *	\brief Sets the global angle of the given L-System to a random value.
 *
 *	\param ls   The L-System for which its global angle should be set to a random value.
 */
void random_angle(lsystem *ls);

/**
 *	\brief Creates a random sequence for the specified rule using only the
 *	specified symbols.
 *
 *	\param ls   The L-System for which one of its rule should be randomized.
 *	\param rule An int representing the rule to randomize.
 */
void random_rule(lsystem *ls, int rule);

/**
 *	\brief Performs the crossover function on the specified parents to produce two children.
 *
 *	\param p1 A pointer to the first parent.
 *	\param p2 A pointer to the second parent.
 *	\param c1 A pointer to where the first child is to be stored.
 *	\param c2 A pointer to where the second child is to be stored.
 */
void crossover_ls(const lsystem *p1, const lsystem *p2, lsystem *c1, lsystem *c2);

/**
 *	\brief Saves the specified L-System to file.
 *
 *	\param ls       An L-System.
 *	\param filename The name of the file to write to.
 *	\param lsname   The name to assign to the L-System.
 *	\param comments Any additional comments to be added to the created files.
 */
void savetofile(lsystem *ls, char *filename, char *lsname, char *comments);

/**
 *	Reads all the L-Systems from the specified file, creates an array of lsystem
 *	objects and saves the L-Systems in the array which is returned via a parameter.
 *
 *	\param fp     A function pointer to the file containing L-Systems.
 *	\param pop    A pointer to an array of lsystem* structures. The created L-Systems
 *	              are returned via this parameter.
 *	\param names  A pointer to an array of char*. The names of the corresponding
 *	              L-Systems are returned via this parameter.
 */
int readfile(FILE *fp, lsystem ***pop, char ***names);

char *rule2string(ruleobj *rule, int rulelength);

#endif
