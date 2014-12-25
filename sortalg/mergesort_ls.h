/**
 *	\file
 *	\brief Sorts an array of lsystem structures, according to a specified
 *	comparator function, by using the \e Mergesort algorithm.
 *
 *	\author Farhan Ahammed (faha3615@mail.usyd.edu.au)
 */


#ifndef MERGESORT_LS_H
#define MERGESORT_LS_H


#include "../ls/ls.h"

/**
 *	\brief The actual data-type of the elements in the array to be sorted.
 */
typedef lsystem *datatype;

/**
 *	\brief Sorts the array in increasing order and returns \c true (\c 1) if successful.
 *
 *	\param input The array (of iterated_function_system structures) to sort.
 *	\param size  The size of the array \c input.
 *	\param cmp   A pointer to a comparator function which compares two
 *	             datatypes (iterated_function_system structures).
 *	\return \c true (\c 1) if the array was successfully sorted.
 */
int mergesort_ls(datatype *input, int size, int (*cmp)(datatype*, datatype*));

#endif
