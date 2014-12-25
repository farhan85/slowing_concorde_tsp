slowing_concorde_tsp
====================

Memetic algorithm written for my honours research and papers


Files
=====
Refer to "Summary Of Files.txt" to see a brief explanation of all the files in this
package.


Installation instructions
=========================

You will need to obtain a copy of the Concorde source code, replace the file
TSP/concorde.c with my modified version, then compile and build (refer to the
Concorde README for help on how to compile and build). Note that you will also
need a linear programming problem solver (I used the freely available 'qsopt')
and if you want to run in Windows, you will need a Cygwin environment.

You will then need to ensure the concorde executable is located in the system
path (i.e. typing 'concorde.exe' at the command prompt should allow the program
to be run).

Use the Makefile to create any of the programs I created. All compiled binaries
will be stored in the ./bin/ directory.

The Makefile can be used to create any of the following programs:

evoalg
	The evolutionary algorithm that evolves instances of L-Systems.

analysels
	A program I created to help 'analyse' an L-System. It computes the upper bound
	and then allows the user to test the upper bound on instances of other sizes.
	Also, the user can specify an arbitrary order and this program will run
	Concorde 15 times and compute the average and standard deviations of the
	average running times and number of branch and bound nodes created.

ls2tsp
	A program which creates a TSPlib file described by a specified L-System.

agent
	A program that performs a local search by perturbing an L-System in different
	ways.

tspsol2mp
	Computes the total cost of a tour and creates PostScript file (using METAPOST)
	of a graphical representation of the tour.

	Note: There are TWO DIFFERENT WAYS TO DESCRIBE THE SOLUTION. One is to use an
	adjacency list (of edges) and the other is to have a list of cities. By default
	the program assumes the solution file is a list of cities.


For ``evoalg`` and ``tspsol2mp``, you can supply the '-h' option to view a list of all the options
that are available.

For ``ls2tsp``, ``analysels`` and ``agent``, run the program with NO arguments to view an explanation
of what arguments are needed and in what order.



