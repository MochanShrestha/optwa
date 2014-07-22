
#include <iostream>
#include <time.h>
using namespace std;

#include "decommon.h"
#include "Cache.h"

// Global variables passed from the user in the command line
int cacheSize;

// Main decomposition function
void decompWorkload (char* infile, char* outfile, int range);
// Process the data and apply the cache filter
void process_cache_filter();

void main(int argc, char* argv[])
{
	// tracefile:	the input trace file
	// outfile:		the output trace file that has the decomposed workload with classes
	// range:		the range of the IRR sub-workload in the decomposition

	if (argc <= 4)
	{
		cout << "USAGE: <tracefile> <outfile> cachesize range" << endl;
		return;
	}

	// Get the cache size and the range
	cacheSize = atoi(argv[3]);
	range = atoi(argv[4]);

	// Timing
	clock_t start = clock();

	// Output the parameters of the program
	cout << "Workload Splitter, Sub-Workload Creator" << endl;
	cout << "---------------------------------------" << endl;
	cout << "Input: " << argv[1] << endl;
	cout << "Output: " << argv[2] << endl;
	cout << "Cache Size: " << cacheSize << endl;
	cout << "Range: " << range << endl;
	cout << "---------------------------------------" << endl;

	// Do the workload decomposition
	decompWorkload (argv[1], argv[2], range);

	clock_t end = clock();
	double time_elapsed_in_seconds = (end - start)/(double)CLOCKS_PER_SEC;
	cout << endl << "Took " << time_elapsed_in_seconds << "s" << endl;
}

void decompWorkload (char* infile, char* outfile, int range)
{
	// Read the trace file to memory
	read_trace_file_TRACE1 (infile);

	// Mark the items that would be filtered by the cache
	cout << "Processing cache filter ... ";
	process_cache_filter();
	cout << " Done!" << endl;

	// Set the sectors that are static on the trace
	cout << "Processing static sectors ...";
	process_static_sectors();
	cout << "Done!" << endl;

	//write_nonstatic_file_TRACE1 ("traced2.dat");
	//return;

	// Calculate the TIs of the remaining workload
	cout << "Calculating TIs ...";
	calculate_TIs();
	cout << "Done!" << endl;

	//write_trace_file_TRACE2TI ("traceti2.dat");
	//return;

	// Decompose the workload and mark it as belonging to which sub-workload
	cout << "Decomposing Workload ...";
	decompose_workload();
	cout << "Done!" << endl;

	// Write the decomposed workload trace file out to file
	write_trace_file_TRACE3 (outfile);

	// Clean up
	clean_up();
}

void process_cache_filter()
{
	// The cache object that will filter the workload for the cache
	Cache c(cacheSize);

	// Event objects that will be used in the loop
	// new event to be added to the cache.
	Event ne;
	// return event
	Event e, re;

	for (int i=0; i<l; i++)
	{
		// Fill up the sector information with the sector stored and the time it was used for
		ne.sector = sector[i];
		ne.itime = i;

		// flag 
		int newEvent = 0;

		// ------------------------------------------------------------------------------
		// Cache Processing
		// ------------------------------------------------------------------------------

		// Check if we are using the cache or not
		if ( c.isUsed() )
		{
			if ( c.process (ne, re) )
			{
				e = re;
				newEvent = 1;
			}
		}
		else
		{
			// Since we are not using the cache, just write the event directly to disk
			e = ne;
			newEvent = 2;
		}

		// ------------------------------------------------------------------------------
		// Event Processing
		// ------------------------------------------------------------------------------

		// Check if there is a new event
		if ( newEvent )
		{
			// We have to mark that this item was not put in the cache
			int ne_i = e.itime;
			regions[ne_i] = 1;
		}
	}

	il = 0;
	for (int i=0; i<l; i++)
	{
		if ( regions[i] == 1 )
			il++;
	}
	cout << "Workload length: " << il << endl;
}