
#include "process.h"

//bool _isWL1 (int op) { return ti[op] <= range; }
bool _isWL1 (int op) { return indices[op] >= 0; }
//bool _isWL2 (int op) { return ti[op] > range; }
bool _isWL2 (int op) { return indices[op] < 0; }

void write_trace_file (char* outfile1, char* outfile2)
{
	FILE* of1;
	fopen_s (&of1, outfile1, "w+");
	FILE* of2;
	fopen_s (&of2, outfile2, "w+");

	fprintf (of1, "%s\n%d\n%d\n%d\n\n", "TRACE2", m, il+wl1_dels, il);
	fprintf (of2, "%s\n%d\n%d\n%d\n\n", "TRACE2", m, (l-il)+wl2_dels, l-il);

	for (int i=0; i<l; i++)
	{	
		if ( _isWL1(i) )
		{
			//Workload1

			fprintf (of1, "%d\t%d\n", 1, sector[i]);

			// Check if we need to write del to workload 2
			// if the previous op was in workload 2, then we have to write del

			// this is the first instance of this sector. Do nothing. Will pop it off next time
			if ( dela[i] == 1 )
			{
				fprintf (of2, "%d\t%d\n", 2, sector[i]);
			}		
		}
		else
		{
			//Workload2

			fprintf (of2, "%d\t%d\n", 1, sector[i]);

			// Check if we need to write del to workload 1
			if ( dela[i] == 2 )
			{
				fprintf (of1, "%d\t%d\n", 2, sector[i]);
			}
		}
	}

	fclose(of1);
	fclose(of2);
}

void process_dels ()
{
	dela = new int[l];
	memset (dela, 0, sizeof(int)*l);
	
	for (int i=0; i<l; i++)
	{
		int c_sector = sector[i];
		queue<int>* cqueue = &(sectori[c_sector]);

		if ( _isWL1(i) )
		{
			//Workload1

			il++;

			// Check if we need to write del to workload 2
			// if the previous op was in workload 2, then we have to write del

			// this is the first instance of this sector. Do nothing. Will pop it off next time
			if ( cqueue->front() == i )
			{
			}
			else
			{
				int prev_op = cqueue->front();
				if ( _isWL2(prev_op) )
				{
					wl2_dels++;
					dela[i] = 1;
				}
				cqueue->pop();
			}
		}
		else
		{
			//Workload2

			// Check if we need to write del to workload 1
			if ( cqueue->front() == i )
			{
			}
			else
			{
				int prev_op = cqueue->front();
				if ( _isWL1(prev_op) )
				{
					wl1_dels++;
					dela[i] = 2;
				}
				cqueue->pop();
			}
		}
	}
}

void build_sector_map()
{
	sectori = new queue<int>[m];
	for (int i=0; i<l; i++)
	{
		sectori[ sector[i] ].push(i);

		if ( ti[i] > range )
		{
			indices[i] = -1;
		}
	}
}

void process3 (char* infile, char* outfile1, char* outfile2, int _range)
{
	// Read the input file
	read_trace_file (infile);

	range = _range;
	wl1_dels = 0;
	wl2_dels = 0;
	il = 0;

	// Make the map of the input file
	cout << "Creating map ... ";
	build_sector_map();
	cout << "done" << endl;

	// Figure out how many in wl1 and how many in wl2
	cout << "Processing workload ...";
	process_dels();
	cout << "done" << endl;
	cout << "WL1_dels: " << wl1_dels << "\tWL2_dels: " << wl2_dels << endl;

	// Write out the output files
	cout << "Writing output files ...";
	write_trace_file (outfile1, outfile2);
	cout << "done" << endl;

	//-----------------------------------------------------------------------
	//------------ CLean up -------------------------------------------------
	//-----------------------------------------------------------------------
	clean_up();
	delete[] dela;
	delete[] sectori;
}