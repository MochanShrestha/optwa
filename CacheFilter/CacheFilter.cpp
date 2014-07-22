
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;

#include <stdlib.h>

#include "Cache.h"
#include "TRACE12Workload.h"

//void process ( char* in, char* out, double cacheProp);
void process ( char* in, char* out, int cacheSize);

void main(int argc, char* argv[])
{
	cout << "Outputs the workload of a trace file after passing through a Cache filter" << endl;

	if (argc <= 2)
	{
		cout << "USAGE: <tracefile> <outfile> <CacheSize>" << endl;
		return;
	}

	//double cacheProp = atof(argv[3]);
	int cacheSize = atoi(argv[3]);

	process (argv[1], argv[2], cacheSize);
}

void process ( char* in, char* out, int cacheSize)
{
	// ------------------------------------------------------------------------------
	// Initalize the Cache and set up the variables
	// ------------------------------------------------------------------------------
	TRACE12Workload* w = new TRACE12Workload (in);
	
	cout << "CacheSize: " << cacheSize << endl;
	Cache c(cacheSize);
	
	FILE* outfile;
	fopen_s (&outfile, out, "w+");
	fprintf (outfile, "TRACE1\n%d\n%d\n\n", w->getM(), w->getl());

	// This is the set of variables that we will keep for statistics
	int nEvents = w->getl();
	int nWritten = 0;

	// ------------------------------------------------------------------------------
	// Run through the workload. Read and write the workload
	// ------------------------------------------------------------------------------
	// Iterate over all the items in the workload
	for (long long i=0; i<w->getl(); i++)
	{
		// Get the next sector
		Event ne = w->getNext();
		Event e, re;
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
			fprintf (outfile, "%d\n", e.sector);
			nWritten++;
		}

		//cout << e;
	}

	// ------------------------------------------------------------------------------
	// Clean up
	// ------------------------------------------------------------------------------
	fclose(outfile);

	// Write up the output message and the statistics
	cout << "Done!" << endl;
	cout << "No. of events: " << nEvents << endl;
	cout << "No. written  : " << nWritten << endl;
	cout << "No. filtered : " << nEvents - nWritten;
	cout << " (";
	cout << std::fixed << std::setprecision(1) << (double)(nEvents - nWritten) / nEvents * 100.0;
	cout << "%)";
	cout << endl;
}