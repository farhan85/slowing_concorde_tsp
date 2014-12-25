/**
 *	\file
 *	\brief Computes an upper or lower bound y = f(x) on a set of (x,y) data points.
 *
 *	\author Farhan Ahammed (faha3615@mail.usyd.edu.au)
 */

#ifndef GRAPH_BOUND_H
#define GRAPH_BOUND_H


/**
 *    \brief A simple (x.y) data point.
 */
typedef struct
{
    /**
     *    \brief The x-value of this data point.
     */
    double x;
    /**
     *    \brief The y-value of this data point.
     */
    double y;
} data_point;


/**
 *	\brief Computes an upper bound y = f(x) on a set of (x,y) data points
 *
 *	During our calculation we 'shift' the data points towards the
 *	origin and scale the points so that\n
 *	<i>(x<sub>0</sub>,y<sub>0</sub>) = (0,0)</i> and\n
 *	<i>(x<sub>1</sub>,y<sub>1</sub>) = (1,1)</i>\n
 *	These numbers will then be incorporated into the upper
 *	bound function so that it will be shifted and scaled back towards where the
 *	data points were in the first place.
 *
 *	Call the returned array \c retval. Then\n
 *	<c>retval[0]</c> is the order of the upper bound.\n
 *	<c>retval[1]</c> is the distance every point is moved along the x-axis.\n
 *	<c>retval[2]</c> is the distance every point is moved along the y-axis.\n
 *	<c>retval[3]</c> is the (multiplicative) scale that every point is moved along the x-axis.\n
 *	<c>retval[4]</c> is the (multiplicative) scale every point is moved along the y-axis.\n
 *
 *	The function returned will then be: <i>y = retval[4]((x - retval[1])/retval[3])<sup>retval[0]</sup> + retval[2]</i>\n
 *
 *	Plot some points, run this code, plot the function, then you'll
 *	understand :)
 *
 *	\param func            An array of size 5. The upper bound function is returned in this array.
 *	\param points          The set of (x,y) data points to find an upper bound for.
 *	\param num_data_points The size of the array \c points.
 */
void find_upper_bound(double func[5], const data_point *points, const int num_data_points);



/**
 *	\brief Computes a lower bound y = f(x) on a set of (x,y) data points
 *
 *	During our calculation we 'shift' the data points towards the
 *	origin and scale the points so that\n
 *	<i>(x<sub>0</sub>,y<sub>0</sub>) = (0,0)</i> and\n
 *	<i>(x<sub>1</sub>,y<sub>1</sub>) = (1,1)</i>\n
 *	These numbers will then be incorporated into the upper
 *	bound function so that it will be shifted and scaled back towards where the
 *	data points were in the first place.
 *
 *	Call the returned array \c retval. Then\n
 *	<c>retval[0]</c> is the order of the upper bound.\n
 *	<c>retval[1]</c> is the distance every point is moved along the x-axis.\n
 *	<c>retval[2]</c> is the distance every point is moved along the y-axis.\n
 *	<c>retval[3]</c> is the (multiplicative) scale that every point is moved along the x-axis.\n
 *	<c>retval[4]</c> is the (multiplicative) scale every point is moved along the y-axis.\n
 *
 *	The function returned will then be: <i>y = retval[4]((x - retval[1])/retval[3])<sup>retval[0]</sup> + retval[2]</i>\n
 *
 *	Plot some points, run this code, plot the function, then you'll
 *	understand :)
 *
 *	\param func            An array of size 5. The upper bound function is returned in this array.
 *	\param points          The set of (x,y) data points to find an upper bound for.
 *	\param num_data_points The size of the array \c points.
 */
void find_lower_bound(double func[5], const data_point *points, const int num_data_points);

/**
 *	\brief Computes the sum of the errors of the points from the given upper bound.
 *
 *	The upper bound array is expected to be in the same format as the array returned
 *	in the function find_upper_bound().
 *
 *	The sum of the errors is computed as SUM{i=1,...,num_data_points}{ub(points[i].x) - points[i].y}.
 *
 *	\param ub              An array of size 5 representing the upper bound function.
 *	\param points          The set of (x,y) data points to compute the sum of errors.
 *	\param num_data_points The size of the array \c points.
 *	\return The sum of the errors.
 */
double sum_error(double ub[5], const data_point *points, const int num_data_points);

#endif
