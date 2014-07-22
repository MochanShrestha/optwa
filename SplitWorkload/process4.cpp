
#include "process.h"

// the vector for sorting
std::vector<int> *ivector;

// the iterator that tells us the split point
std::vector<int>::iterator split_point;
// how many operations we have to potentially process
int nToProcess;

void sort_workload()
{
	// Sort the indices
	ivector = new std::vector<int> (indices, indices+l);
	std::sort (ivector->begin(), ivector->end(), ti_compare);

	/*
	// Debug routine. Write out some of the sorted values
	for (std::vector<int>::iterator it=ivector->begin(); it!=ivector->begin()+100; ++it)
	{
		int index = *it;
		//cout << index << endl;
		cout << index << ", " << sector[index] << ", " << ti[index] << endl;
	}
	*/
}

void find_split_point()
{
	nToProcess = 0;

	for (std::vector<int>::iterator it=ivector->begin(); it!=ivector->end(); ++it)
	{
		int index = *it;

		if ( ti[index] < range )
		{
			split_point = it;
			break;
		}

		nToProcess++;
	}
}

void remove_op4 (int op)
{
	// precursors

	int index = indices[op];
	int op_sector = sector[op];
	indices[op] = -1;
	il--;

	//-----------------------------------------------------------------------
	//------------- Find operations before op that have next op with same sector
	//------------------- after the removed op and adjust TI
	//-----------------------------------------------------------------------

	// Loop over the operations before this op
	for (int i=0; i<op; i++)
	{
		if ( isWL1(i) )
		{
			// if any previous operation has IT after this, decrease it's TI by 1
			if ( get_it(i) > index )
				ti[i]--;

			// for calculating max
			//if ( ti[i] > max_val )
			//{
			//	max_val = ti[i];
			//	max_index = i;
			//}
		}
	}

	//-----------------------------------------------------------------------
	//----------- Decrement the indices after op removal --------------------
	//-----------------------------------------------------------------------

	// Loop over the operations after op
	for (int i=op; i<l; i++)
	{
		if ( isWL1(i) )
		{
			// we removed an item from workload1 and so every index after it is changed
			indices[i]--;

			// for calculating max
			//if ( ti[i] > max_val )
			//{
			//	max_val = ti[i];
			//	max_index = i;
			//}
		}
	}
}

void split_workload()
{
	std::vector<int>::iterator it=ivector->begin();

	// percentage complete
	int pComplete = 0;

	for (int i=0; i<nToProcess; i++)
	{
		// Find the index of the operation
		int index = *it;

		if ( indices[index] >=0 && ti[index] >= range )
		{
			remove_op4 ( index );
		}

		int nComplete = (int)( (double)i / (double)nToProcess * 100 );
		if ( nComplete > pComplete )
		{
			pComplete = nComplete;
			cout << pComplete << "% complete" << " (" << i << '/' << nToProcess << ")";
			cout << '\r';
		}

		// We are done processing this operation. Increment
		it++;
	}
}

void build_sector_map4()
{
	sectori = new queue<int>[m];
	for (int i=0; i<l; i++)
	{
		sectori[ sector[i] ].push(i);
	}
}

extern bool _isWL1 (int op);
extern bool _isWL2 (int op); 

void write_trace_file4 (char* outfile1, char* outfile2)
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

void process_dels4 ()
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

void process4 (char* infile, char* outfile1, char* outfile2, int _range)
{
	range = _range;

	// Read the input file
	read_trace_file (infile);

	// sort the largest ti worklods
	cout << "Sorting workload ... ";
	sort_workload();
	cout << "done" << endl;

	// find where we have to split the workload
	cout << "Finding split point ...";
	find_split_point();
	cout << "done. Processing " << nToProcess << '/' << l << " operations. " << endl;

	// Go through the workload and then remove points from the workload
	cout << endl << "Splitting workload ..." << endl;
	split_workload();
	cout << "... done                                 " << endl << endl;

	// Make the map of the input file
	cout << "Creating map ... ";
	build_sector_map4();
	cout << "done" << endl;

	// Figure out where the deletes need to be
	cout << endl << "Processing deletes ... " << endl;
	process_dels4();
	cout << "... done" << endl << endl;

	// Write out the output files
	cout << "Writing output files ...";
	write_trace_file4 (outfile1, outfile2);
	cout << "done" << endl;

	//-----------------------------------------------------------------------
	//------------ CLean up -------------------------------------------------
	//-----------------------------------------------------------------------
	clean_up();
	delete[] dela;
	delete[] sectori;
}