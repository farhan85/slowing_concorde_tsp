/**
 *	\file
 *	\brief Implements the methods defined in the header file graph_bound.h
 *
 *	\author Farhan Ahammed (faha3615@mail.usyd.edu.au)
 */

#include <math.h>
#include <stdlib.h>

#include "upper_bound.h"

/**
 *	\brief Returns \c true (\c 1) if the function <i>x<sup>a</sup></i> lies \e above the data points,
 *	\c false (\c 0) otherwise.
 *
 *	\param  a               The exponent to be tested.
 *	\param  points          The set of data points the exponent is being tested on.
 *	\param  num_data_points The size of the array \c points.
 *	\return \c true (\c 1) or \c false (\c 2) if the function <i>x<sup>a</sup></i> lies above or
 *	        below the data points respectively.
 */
int lies_above_points(const double a, const data_point *points, const int num_data_points) {
	double y = 0, y_hat = 0;
	int i;
	for (i = 0; i < num_data_points; i++)
	{
		if (   (points[i].y == 0) || (points[i].y == 1)
		    || (points[i].x == 0) || (points[i].x == 1))
		{
			y_hat = points[i].y;     /* the actual value    */
			y = pow(points[i].x, a); /* the estimated value */
		}
		else
		{
			y_hat = log(points[i].y); /* the actual value:    log(y)              */
			y = (a)*log(points[i].x); /* the estimated value: log(x^a) = a.log(x) */
		}

		if (y < y_hat)
			return 0;
	}
	return 1;
}

/**
 *	\brief Returns \c true (\c 1) if the function <i>x<sup>a</sup></i> lies \e below the data points,
 *	\c false (\c 0) otherwise.
 *
 *	\param  a               The exponent to be tested.
 *	\param  points          The set of data points the exponent is being tested on.
 *	\param  num_data_points The size of the array \c points.
 *	\return \c true (\c 1) or \c false (\c 2) if the function <i>x<sup>a</sup></i> lies above or
 *	        below the data points respectively.
 */
int lies_below_points(const double a, const data_point *points, const int num_data_points) {
	double y = 0, y_hat = 0;
	int i;
	for (i = 0; i < num_data_points; i++)
	{
		if (   (points[i].y == 0) || (points[i].y == 1)
		    || (points[i].x == 0) || (points[i].x == 1))
		{
			y_hat = points[i].y;     /* the actual value    */
			y = pow(points[i].x, a); /* the estimated value */
		}
		else
		{
			y_hat = log(points[i].y); /* the actual value:    log(y)              */
			y = (a)*log(points[i].x); /* the estimated value: log(x^a) = a.log(x) */
		}

		if (y > y_hat)
			return 0;
	}
	return 1;
}

/**
 *	\brief Returns a number (\e a) such that <i>x<sup>a</sup></i> lies above
 *	all the data points.
 *
 *	Use this method as a starting point.
 *
 *	\return A number (\e a) such that <i>x<sup>a</sup></i> lies above all the data points.
 */
double initial_function(data_point *points, int num_data_points) {
	double a = 5;

	/* If the value of a is not large enough to make the function x^a
	   lie above all the data points, then double it and try again    */
	while (!lies_above_points(a, points, num_data_points)) {
		a = 2*a;
	}
	return a;
}



/**
 *	\brief Compares two data_point structures and determines which one
 *	has the smaller \e x coordinate.
 *
 *	\param struct1 The first data_point structure.
 *	\param struct2 The second data_point structure.
 *	\return -1 if the \e x coordinate of struct1 is less than the \e x coordinate of struct2,
 *	1 if larger, and 0 if they are the same.
 */
int compare_data_points(const void *struct1, const void *struct2)
{
	data_point *dp1 = (data_point *)struct1, *dp2 = (data_point *)struct2;
	if (dp1->x < dp2->x)
		return -1;
	else if (dp1->x > dp2->x)
		return 1;
	else
		return 0;
}


void find_upper_bound(double func[5], const data_point *points, const int num_data_points)
{
	/* You're wasting your time using this method when you have less than 2 points. This
	   method will give you a meainingful result if there are more than 3 data points.   */
	if (num_data_points < 2)
	{
		func[0] = 1;
		func[1] = points[0].x;
		func[2] = points[0].y;
		func[3] = points[1].y - func[1];
		func[4] = points[1].x - func[2];
		return;
	}

	/* Check that we have unique x coordinates (ie. we have a mathematical function).
	   This algorithm won't work otherwise.                                           */
	data_point *_points = (data_point *)malloc(sizeof(data_point)*num_data_points);
	int i;
	for (i = 0; i < num_data_points; i++)
	{
		_points[i].x = points[i].x;
		_points[i].y = points[i].y;
	}
	qsort(_points, num_data_points, sizeof(data_point), &compare_data_points);
	for (i = 1; i < num_data_points; i++)
	{
		if (_points[i].x == _points[i-1].x) /* We don't have a mathematical function. */
		{
			func[0] = 0;
			func[1] = points[0].x;
			func[2] = points[0].y;
			func[3] = points[1].y - func[1];
			func[4] = points[1].x - func[2];
			return;
		}
	}

	/* Shift the points towards the origin. */
	func[1] = _points[0].x;
	func[2] = _points[0].y;

	/* Scale the points to a polynomial of the form y = x^a (so that there is only one
	   constant we have to worry about - i.e. the 'a').                                */
	func[3] = _points[1].x - func[1];
	if (func[3] == 0) func[3] = 1;

	func[4] = _points[1].y - func[2];
	if (func[4] == 0) func[4] = 1;

	/* Now perform the actual shift and scaling. */
	for (i = 0; i < num_data_points; i++)
	{
		_points[i].x = (_points[i].x - func[1])/func[3];
		_points[i].y = (_points[i].y - func[2])/func[4];
	}

	/* We are trying to find a polynomial of the form x^a */
	double ahigh, alow, atemp, error;

	/* Get an initial function */
	ahigh = initial_function(_points, num_data_points);
	alow = 0;

	do {
		/* Use the `divide & conquer' approach to find a good upper bound estimate. */
		atemp = (ahigh + alow)/2;
		if (lies_above_points(atemp, _points, num_data_points))
			ahigh = atemp;
		else
			alow = atemp;
		error = ahigh - alow;
	}while (error > 0.0005);

	func[0] = ahigh;
	free(_points);
}


void find_lower_bound(double func[5], const data_point *points, const int num_data_points)
{
	/* You're wasting your time using this method when you have less than 2 points. This
	   method will give you a meainingful result if there are more than 3 data points.   */
	if (num_data_points < 2)
	{
		func[0] = 1;
		func[1] = points[0].x;
		func[2] = points[0].y;
		func[3] = points[1].y - func[1];
		func[4] = points[1].x - func[2];
		return;
	}

	/* Check that we have unique x coordinates (ie. we have a mathematical function).
	   This algorithm won't work otherwise.                                           */
	data_point *_points = (data_point *)malloc(sizeof(data_point)*num_data_points);
	int i;
	for (i = 0; i < num_data_points; i++)
	{
		_points[i].x = points[i].x;
		_points[i].y = points[i].y;
	}
	qsort(_points, num_data_points, sizeof(data_point), &compare_data_points);
	for (i = 1; i < num_data_points; i++)
	{
		if (_points[i].x == _points[i-1].x) /* We don't have a mathematical function. */
		{
			func[0] = 0;
			func[1] = points[0].x;
			func[2] = points[0].y;
			func[3] = points[1].y - func[1];
			func[4] = points[1].x - func[2];
			return;
		}
	}

	/* Shift the points towards the origin. */
	func[1] = _points[0].x;
	func[2] = _points[0].y;

	/* Scale the points to a polynomial of the form y = x^a (so that there is only one
	   constant we have to worry about - i.e. the 'a').                                */
	func[3] = _points[1].x - func[1];
	if (func[3] == 0) func[3] = 1;

	func[4] = _points[1].y - func[2];
	if (func[4] == 0) func[4] = 1;

	/* Now perform the actual shift and scaling. */
	for (i = 0; i < num_data_points; i++)
	{
		_points[i].x = (_points[i].x - func[1])/func[3];
		_points[i].y = (_points[i].y - func[2])/func[4];
	}

	/* We are trying to find a polynomial of the form x^a */
	double ahigh, alow, atemp, error;

	/* Get an initial function */
	ahigh = initial_function(_points, num_data_points);
	alow = 0;

	do {
		/* Use the `divide & conquer' approach to find a good upper bound estimate. */
		atemp = (ahigh + alow)/2;
		if (lies_below_points(atemp, _points, num_data_points))
			alow = atemp;
		else
			ahigh = atemp;
		error = ahigh - alow;
	}while (error > 0.0005);

	func[0] = alow;
	free(_points);
}


double sum_error(double ub[5], const data_point *points, const int num_data_points)
{
	double error = 0;
	int i;
	for (i = 0; i < num_data_points; i++)
	{
		error = error + pow((points[i].x - ub[1])/ub[3], ub[0])*ub[4] + ub[2] - points[i].y;
	}
	return error;
}
