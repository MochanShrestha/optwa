

#include "common.h"
#include "process.h"

#include <time.h>

void main(int argc, char* argv[])
{
	if (argc <= 4)
	{
		cout << "USAGE: <tracefile> <outfile1> <outfile2> range" << endl;
		return;
	}

	int range = atoi(argv[4]);

	// Timing
	clock_t start = clock();

	// Output the parameters of the program
	cout << "Workload Splitter, Sub-Workload Creator" << endl;
	cout << "---------------------------------------" << endl;
	cout << "Input: " << argv[1] << endl;
	cout << "Output1: " << argv[2] << endl;
	cout << "Output2: " << argv[3] << endl;
	cout << "Range: " << range << endl;
	cout << "---------------------------------------" << endl;

	process3 (argv[1], argv[2], argv[3], range);

	clock_t end = clock();
	double time_elapsed_in_seconds = (end - start)/(double)CLOCKS_PER_SEC;
	cout << endl << "Took " << time_elapsed_in_seconds << "s" << endl;
}

