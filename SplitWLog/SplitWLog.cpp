
#include <iostream>
#include <time.h>
using namespace std;

#include "WorkloadSplitter.h"

void main (int argc, char** argv)
{
	cout << "Log Workload Splitter" << endl;

	if ( argc < 5 )
	{
		cout << "Usage: <tracefile>" << endl;
		exit(-1);
	}

	WorkloadSplitter ws ( argv[1] );
	int range = atoi ( argv[4] );
	
	// Timing routines
	clock_t start_time = clock();

	ws.split ( range, argv[2], argv[3] );
	//ws.split_l2 ( range, argv[2], argv[3] );

	// Timing routines
	clock_t end = clock();
	double time_elapsed_in_seconds = (end - start_time)/(double)CLOCKS_PER_SEC;
	cout << endl << "Took " << time_elapsed_in_seconds << "s" << endl;
}