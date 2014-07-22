
#include "process.h"

#define WORKLOAD2_DELOP 1

// finds the workload with the max ti value in workload1
int find_max()
{
	int max_val = -1;
	int max_index = -1;

	for (int i=0; i<l; i++)
	{
		// check it's workload 1
		if ( isWL1(i) )
		{
			if ( ti[i] > max_val )
			{
				max_val = ti[i];
				max_index = i;
			}
		}
	}

	return max_index;
}

// removes the operation from workload1
int removeOp(int op)
{
	//cout << "Removing " << op << ", index=" << indices[op] << ", sector=" << sector[op] << ", ti=" << ti[op] << endl;

	// for calculating the max_ti
	int max_val = -1;
	int max_index = -1;

	// make sure we are removing from workload1
	if ( indices[op] < 0 )
	{
		cout << "ERROR: Trying to remove from workload2. (" << op << "," << sector[op] << "," << ti[op] << ")" << endl;
		exit(-1);
	}

	//-----------------------------------------------------------------------
	//------------------- Variables -----------------------------------------
	//-----------------------------------------------------------------------

	// mark the op as now being in workload 2
	int index = indices[op];
	int op_sector = sector[op];
	indices[op] = -1;
	il--;

	// for checking if a previous operation was for the same sector
	int prev_op = -1;
	int next_op = -1;

	//-----------------------------------------------------------------------
	//------------- Find operations before op that have next op with same sector
	//------------------- after the removed op and adjust TI
	//------------- Find the prev op has the same sector
	//-----------------------------------------------------------------------

	// Loop over the operations before this op
	for (int i=0; i<op; i++)
	{
		if ( isWL1(i) )
		{
			// if any previous operation has IT after this, decrease it's TI by 1
			if ( get_it(i) > index )
				ti[i]--;

			// check if a previous operation references the same sector
			if ( sector[i] == op_sector )
			{
				prev_op = i;
				//cout << "Found one! " << sector[op] << " in " << op << " and " << prev_op << endl;
			}

			// for calculating max
			if ( ti[i] > max_val )
			{
				max_val = ti[i];
				max_index = i;
			}
		}
	}

	//-----------------------------------------------------------------------
	//----------- Decrement the indices after op removal --------------------
	//----------- Find the next operation with the same sector --------------
	//-----------------------------------------------------------------------

	// Loop over the operations after op
	for (int i=op; i<l; i++)
	{
		if ( isWL1(i) )
		{
			// we removed an item from workload1 and so every index after it is changed
			indices[i]--;

			// for calculating max
			if ( ti[i] > max_val )
			{
				max_val = ti[i];
				max_index = i;
			}
		}

		// check if a future op is the same sector
		if ( sector[i] == op_sector && next_op < 0 )
		{
			next_op = i;
			//cout << "Found one! " << sector[op] << " in " << op << " and " << prev_op << endl;
		}
	}

	//-----------------------------------------------------------------------
	//-------------- Working with previous operation and next operation -----
	//-----------------------------------------------------------------------

	// If the op removed was the TI of a previous workload, fix it
	if ( prev_op > 0 )
	{
		// mark it as needing to produce a del operation. Will have to be double checked during writing
		indices[op] = -1 - prev_op;

		/*
		// Find the next op that has the same sector on it
		for (int i=op+1; i<l; i++)
		{
			if ( isWL1(i) )
			{
				if ( sector[i] == sector[prev_op] )
				{
					found = true;
					ti[prev_op] = indices[i] - indices[prev_op];
				}
			}
		}

		// Not found. Point to the end of the workload
		if (found == false)
		{
			ti[prev_op] = il-indices[prev_op];
		}*/
	}

	if ( next_op > 0 )
	{
		if ( isWL1(next_op) )
		{
			flag[next_op] = WORKLOAD2_DELOP;
			cout << "Setting up a Del Op for workload2" << endl;
		}
	}

	return max_index;
}

void splitWorkload2 (int range)
{
	// find the index of the max index
	int maxti_index = find_max();

	// Remove ops from workload1 and place to workload 2
	while (1)
	{
		// The max ti in the workload is now less than the range. We are done
		if ( ti[maxti_index] <= range )
		{
			break;
		}

		// remove the operation from the workload
		maxti_index = removeOp ( maxti_index );
	}
}

void process2 (char* infile, char* outfile1, char* outfile2, int range)
{
	//-----------------------------------------------------------------------
	//----------- Read the input workload into memory -----------------------
	//-----------------------------------------------------------------------
	read_trace_file (infile);

	//-----------------------------------------------------------------------
	// ---------- Separate the workload -------------------------------------
	//-----------------------------------------------------------------------
	cout << "Splitting Workload ... ";

	splitWorkload2(range);

	cout << "done" << endl;

	//-----------------------------------------------------------------------
	// Find how many delete operations were added to Workload 1
	//-----------------------------------------------------------------------
	int wl1_dels = 0;
	for (int i=0; i<l; i++)
	{
		if ( indices[i] < -1 )
		{
			int prev_index = -indices[i] - 1;
			if ( isWL1(prev_index) )
			{
				wl1_dels++;
			}
		}
	}

	//-----------------------------------------------------------------------
	// ---------- Output the workload to two separate files  ----------------
	//-----------------------------------------------------------------------
	cout << "Writing Split Workloads ... ";

	/*ofstream ofile1(outfile1), ofile2(outfile2);

	// Write out the headers for both the files
	ofile1 << "TRACE2TI" << endl;
	ofile2 << "TRACE2TI" << endl;
	ofile1 << m << endl;
	ofile2 << m << endl;
	ofile1 << il << endl;
	ofile2 << (l - il) << endl;
	ofile1 << endl;
	ofile2 << endl;

	// Write out the contents of the files
	for (int i=0; i<l; i++)
	{
		if ( isWL1(i) )
		{
			ofile1 << 1 << '\t' << sector[i] << '\t' << ti[i] << endl;
		}
		else
		{
			//ofile2 << sector[i] << '\t' << ti[i] << endl;
			ofile2 << 1 << '\t' << sector[i] << endl;
		}
	}

	ofile1.close();
	ofile2.close();*/

	// Output the files with the C implementation (hoping it's faster)
	FILE* of1 = fopen(outfile1, "w+");
	FILE* of2 = fopen(outfile2, "w+");

	fprintf (of1, "%s\n%d\n%d\n%d\n\n", "TRACE2TI", m, il+wl1_dels, il);
	fprintf (of2, "%s\n%d\n%d\n\n", "TRACE2", m, (l-il));

	for (int i=0; i<l; i++)
	{
		if ( isWL1(i) )
		{
			fprintf (of1, "%d\t%d\t%d\n", 1, sector[i], ti[i]);
			if ( flag[i] == WORKLOAD2_DELOP )
			{
				fprintf (of2, "%d\t%d\n", 2, sector[i]);
			}
		}
		else
		{
			// Check if this needs to produce a delete operation
			if ( indices[i] < -1 )
			{
				int prev_index = -indices[i] - 1;
				if ( isWL1(prev_index) )
				{
					fprintf (of1, "%d\t%d\n", 2, sector[i]);
				}
			}

			fprintf (of2, "%d\t%d\n", 1, sector[i]);
		}
	}

	fclose(of1);
	fclose(of2);

	cout << "done" << endl;

	//-----------------------------------------------------------------------
	//------------ CLean up -------------------------------------------------
	//-----------------------------------------------------------------------
	clean_up();
}