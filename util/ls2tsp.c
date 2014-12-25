/**
 *	\file
 *	\brief Creates a TSPlib plot of the specified L-System.
 *
 *	\author Farhan Ahammed (faha3615@mail.usyd.edu.au)
 */

#include <stdio.h>
#include <stdlib.h>


/**
 *	\brief Creates a TSPlib plot of the specified L-System.
 *
 *	You \e must delete the individual strings and the array itself when finished
 *	with them.
 *
 *	\param filename The name to give to the TSPlib file.
 *	\param order    The order of the fractal to create.
 *	\param lsysfile The file containing the L-System to use, to generate the TSPlib files.
 *	\param lsname   The name of the L-System to use.
 */
void createOneTSPFile(char *filename, int order, const char *lsysfile, const char *lsname)
{
	/* Compute the x-y coordinates of the instance represented by this L-System. */
	int numnodes = Lsystem(order, lsysfile, lsname, "plots");

	/* Now create a valid TSPlib file. */
	FILE *outfile_stream = fopen(filename, "w");

	/* Check that the file has sucessfully been opened */
	if (outfile_stream == NULL)
	{
		printf("ERROR: File '%s' could not be opened\n", filename);
		return;
	}

	/* Build the header file */
	fprintf(outfile_stream, "NAME: %s\n", lsname);
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

	fprintf(outfile_stream, "EOF:\n");

	/* Close the file */
	int close_error_plot = fclose(outfile_stream);
	if (close_error_plot)
	{
		printf("ERROR: File '%s' could not be closed\n", filename);
	}
}



/**
 *	\brief Starts the program.
 *
 *	One argument is expected. The program is run like this:
 *	\code
 *  <program name> <ifs file>
 *	\endcode
 */
int main(int argc, char** argv)
{
	if (argc < 4)
		printf("usage:\n%s <L-System file> <L-System name> <order> <output file name>\n", argv[0]);
	else
		createOneTSPFile(argv[4], atoi(argv[3]), argv[1], argv[2]);
	return 0;
}
