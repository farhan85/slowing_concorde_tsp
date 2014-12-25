/**
 *	\file
 *	\brief Sorts an array of lsystem structures, according to a specified
 *	comparator function, by using the \e Mergesort algorithm.
 *
 *	Most parts of this code was obtained from\n
 *	http://www.cprogramming.com/tutorial/computersciencetheory/mergesort.html
 *
 *	\author Farhan Ahammed (faha3615@mail.usyd.edu.au)
 */

#include "mergesort_ls.h"


/**
 *	\brief Helper function to sort the array.
 *
 *	\param input   The array (of iterated_function_system structures) to sort.
 *	\param left    The index of the leftmost element of the subarray.
 *	\param right   One past the index of the rightmost element.
 *	\param scratch An array of the same length as \c input.
 *	\param cmp     A pointer to a comparator function which compares two
 *	               datatypes (iterated_function_system structures).
 */
void merge_helper(datatype *input, int left, int right, datatype *scratch, int (*cmp)(datatype*, datatype*))
{
    /* base case: one element */
    if(right == left + 1)
    {
        return;
    }
    else
    {
        int i = 0;
        int length = right - left;
        int midpoint_distance = length/2;
        /* l and r are to the positions in the left and right subarrays */
        int l = left, r = left + midpoint_distance;

        /* sort each subarray */
        merge_helper(input, left, left + midpoint_distance, scratch, cmp);
        merge_helper(input, left + midpoint_distance, right, scratch, cmp);

        /* merge the arrays together using scratch for temporary storage */
        for(i = 0; i < length; i++)
        {
            /* Check to see if any elements remain in the left array; if so,
               we check if there are any elements left in the right array; if
               so, we compare them.  Otherwise, we know that the merge must
               take the element from the left array */
            if(l < left + midpoint_distance &&
                    (r == right || cmp(&input[l], &input[r]) < 0))
            {
                scratch[i] = input[l];
                l++;
            }
            else
            {
                scratch[i] = input[r];
                r++;
            }
        }

        /* Copy the sorted subarray back to the input */
        for(i = left; i < right; i++)
        {
            input[i] = scratch[i - left];
        }
    }
}



int mergesort_ls(datatype *input, int size, int (*cmp)(datatype*, datatype*))
{
    datatype *scratch = (datatype *)malloc(size * sizeof(datatype));
    if(scratch != NULL)
    {
        merge_helper(input, 0, size, scratch, cmp);
        free(scratch);
        return 1;
    }
    else
    {
        return 0;
    }
}
