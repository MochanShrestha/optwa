
#include "common.h"

// no. of sectors
int m;
// length of the workload
int l;
int il;
// the length of il before decomposing
int _il;
// number of static sectors
int statics;

// array of which sector it is being written to
int* sector;
// the time to invalidation of the sector
int* ti;
int* indices;
int* flag;
int* regions;

int range;

// how the sectors are accessed
queue<int>* sectori;

// the number of deletes for workload 1 and workload 2
int wl1_dels;
int wl2_dels;

int* dela;

// For the removal candidates
int* rv_can;
int rv_can_count;
// End of the selection from the candidates
int rv_end;


FILE* tfd;
int tio_count;


void clean_up()
{
	delete[] indices;
	delete[] sector;
	delete[] ti;
	delete[] flag;
	delete[] regions;

	delete[] sectori;
}

bool ti_compare (int i,int j) 
{
	return (ti[i]>ti[j]); 
}

// get the IT (invalidation time) from TI (time to invalidation)
int get_it (int i)
{
	return indices[i] + ti[i];
}


void read_trace_file_TRACE1 (char* infile)
{
	// the input file object to read from
	FILE* inputfile;
	fopen_s (&inputfile, infile, "r");

	// Read the header
	char header[255];
	fscanf(inputfile, "%s\n%d\n%d\n\n", header, &m, &l);
	cout << "Inputfile: ";
	cout << "Format: " << header << ", m=" << m << ", l=" << l << endl;

	// Make sure we have the right file format
	if ( strcmp (header, "TRACE1") != 0 )
	{
		cout << "File format is not TRACE1" << endl;
		exit(-1);
	}

	// Read the file into memory
	cout << "Reading file to memory ... ";

	sector = new int[l];
	for (int i=0; i<l; i++)
	{
		fscanf (inputfile, "%d\n", sector+i);
		if ( feof (inputfile)  )
		{
			if ( i != l - 1 )
			{
				cout << "ERROR! Workload length given does not match file.";
				cout << "l=" << l << ", i=" << i;
				cout << endl;
				exit(-1);
			}
		}
	}

	cout << " done" << endl;

	// ... Testing. Print out a few of the values read
	//for (int i=l-10; i<l; i++)
	//{
	//	cout << i << ", " << sector[i] << ", " << ti[i] << endl;
	//}

	// fill up the array of indices
	indices = new int[l];
	for (int i=0; i<l; i++)
	{
		indices[i] = i;
	}
	il = l;

	// This is for storing flags regarding the operations
	flag = new int[l];
	memset (flag, 0, sizeof(int)*l);

	// This is to set the regions that each operation will be in
	regions = new int[l];
	for (int i=0; i<l; i++)
	{
		regions[i] = 0;
	}

	fclose(inputfile);
}



void process_static_sectors()
{
	// --------------------------------------------------------------------
	// ---- Create the array of information about the sectors
	// --------------------------------------------------------------------
	cout << "Creating information about the sectors ... ";
	int* op_count = new int[m];
	int* op_last  = new int[m];

	memset (op_count, 0, sizeof(int)*m);

	for (int i=0; i<l; i++)
	{
		if ( regions[i] == 1 )
		{
			op_count [ sector[i] ]++;
			op_last  [ sector[i] ] = i;
		}
	}
	cout << "done" << endl;

	// --------------------------------------------------------------------
	// ---- Process the static sectors
	// --------------------------------------------------------------------
	statics = 0;
	for (int i=0; i<m; i++)
	{
		if ( op_count[i] == 1 )
		{
			statics++;
		}
	}
	cout << "# of statics = " << statics << "/" << m << endl;

	// --------------------------------------------------------------------
	// ---- Mark the static in the static region of -1
	// --------------------------------------------------------------------
	for (int i=0; i<l; i++)
	{
		if ( regions[i] == 1 )
		{
			if ( op_count[sector[i]] == 1)
				regions[i] = -1;
		}
	}
}

void calculate_TIs()
{
	// ------------------------------------------------------------------------
	// --------------------------- Create the data structures
	// ------------------------------------------------------------------------
	queue<int>* sectori = new queue<int>[m];
	ti = new int[l];
	memset(ti, 0, sizeof(int)*l);

	// ------------------------------------------------------------------------
	// ------------------------ Fill up the data structures
	// ------------------------------------------------------------------------
	cout << "Reading file and filling data structures ... ";
	int cur_index = 0;
	for (int i=0; i<l; i++)
	{
		if ( regions[i] == 1 )
		{
			sectori[ sector[i] ].push(i);

			indices[i] = cur_index;
			cur_index++;
		}
		else
		{
			indices[i] = -1;
		}
	}
	il = cur_index;
	cout << "done" << endl;

	// ------------------------------------------------------------------------
	// --------------------------- Calculate the TIs
	// ------------------------------------------------------------------------
	cout << "Calculating TIs ... ";

	// percentage complete
	int pc = 0;

	for (int i=0; i<l; i++)
	{
		if ( regions[i] == 1 )
		{
			int c_sector = sector[i];
			queue<int>* cqueue = &(sectori[c_sector]);

			// if the top of the queue is the current sector, remove it
			if ( cqueue->front() == i )
			{
				sectori[c_sector].pop();
			}
		
			// If there is nothing in the queue, put the tti as the end of the workload
			if ( cqueue->empty() )
			{
				//ti[i] = l - i;
				ti[i] = cur_index - indices[i];
				continue;
			}

			// It's not empty, then set the tti as the next value
			int nexti = cqueue->front();
			//ti[i] = nexti - i;
			ti[i] = indices[nexti] - indices[i];
			cqueue->pop();

			// output the percentage counter
			int pcn = (int)(i/(double)l*100.0f);
			if ( pcn > pc )
			{
				pc = pcn;
				cout << "Percentage complete: " << pc << endl;
			}
		}
	}

	cout << "done" << endl;
}


void setWL2 ( int op )
{
	regions[op] = 2;
}

int linear_search_between (int it)
{
	// Check how many in between and remove them
	int in_btwn = 0;
	for (int i=rv_end-1; i>=0; i--)
	{
		if ( rv_can[i] < it )
		{
			in_btwn++;
		}
		else
		{
			break;
		}
	}

	return in_btwn;
}

int binary_search_between (int it)
{
	if ( rv_end == 1 )
	{
		if ( rv_can[0] >= it )
		{
			return 0;
		}
		else
			return 1;
	}

	int ib_left = 0;
	int ib_right = rv_end-1;
	int ib_center;

	while ( ib_right - ib_left > 1 )
	{
		ib_center = (ib_left + ib_right) / 2;
		if ( rv_can[ib_center] >= it )
		{
			ib_left = ib_center;
		}
		else
		{
			ib_right = ib_center;
		}
	}

	if ( rv_can[ib_left] >= it )
		return (rv_end-ib_right);
	else 
		return (rv_end-ib_left);
}

int get_in_between (int op)
{
	// We have to get the number of removed operations between op and op+ti[op]
	int it = indices[op] + ti[op];

	// If invalidation is at the end of the file, then just give how much we have
	if ( it == _il )
		return rv_end;

	// If nothing has been selected for W2
	if ( rv_end == 0 )
	{
		return 0;
	}

	//return linear_search_between(it);

	// NOTE: My binary search implementation has endpoint problems so just take care of it here.
	// If the last one removed is greater than the it of the op
	if ( rv_can[rv_end-1] >= it )
	{
		return 0;
	}
	if ( rv_can[0] < it )
	{
		return rv_end;
	}

	return binary_search_between(it);
}

void remove_ops_up()
{
	rv_end = 0;

	// percentage stuff
	int p_comp = 0;

	// Iterate through the workload from down to up
	for (int i=l-1; i>=0; i--)
	{
		// Ignore for the operations that are not 
		if ( regions[i] == 1 )
		{
			// Update the TIs for the operation
			int tib = get_in_between (i);
			if ( tib > 0 && tib != rv_end && tio_count < 10000 )
			{
				fprintf (tfd, "%d\t%d\n", indices[i], tib);
				tio_count++;
			}
			ti[i] -= tib;

			// If the ti is larger than the split, remove the operation
			if ( ti[i] > range )
			{
				// Set for workload 2
				setWL2(i);
				il--;

				// Add it to the rv_can array
				rv_can[rv_end] = indices[i];
				rv_end++;
			}



			// Stuff for calculating the percentage
			int p_comp_now = (int)((double)(l-1-i)*100.0/l);
			if ( p_comp_now > p_comp )
			{
				p_comp = p_comp_now;
				cout << "\t" << p_comp << "% complete." << endl;
			}
		}
	}
}

void decompose_workload()
{
	cout << endl;
	cout << "Starting Split ... " << endl;

	cout << '\t' << "Split point: " << range << endl;

	// The indices into the sector array that tells us the candidates we can remove
	rv_can = new int[l];
	_il = il;

	fopen_s (&tfd, "deltati2.txt", "w");
	tio_count = 0;

	// Remove all the processing and remove all the operations
	remove_ops_up();

	cout << "... done" << endl;

	// Clean up
	delete[] rv_can;

	// Make the map of the input file
	// -- This is already done when calculating the TI so we don't do it again. We keep the old structure around.
	//cout << "Creating map ... ";
	//build_sector_map();
	//cout << "done" << endl;

	// Figure out where the deletes need to be
	// -- We do not need to process deletes becase we are not really making multiple workloads but just marking regions
	//cout << endl << "Processing deletes ... " << endl;
	//process_dels();
	//cout << "... done" << endl << endl;
}



void write_trace_file_TRACE3 (char* outfile)
{
	FILE* ofd = fopen (outfile, "w");

	fprintf (ofd, "%s\n%d\n%d\n\n", "TRACE3", m, l);
	
	for (int i=0; i<l; i++)
	{
		fprintf (ofd, "%d\t%d\n", sector[i], regions[i]);
	}
	
	fclose (ofd);
}

void write_trace_file_TRACE2TI (char* outfile)
{
	FILE* ofd = fopen (outfile, "w");

	il = 0;
	for (int i=0; i<l; i++)
	{
		if ( regions[i] == 1 )
			il++;
	}

	fprintf (ofd, "%s\n%d\n%d\n\n", "TRACE1TI", m, il);
	
	for (int i=0; i<l; i++)
	{
		if ( regions[i] == 1 )
			fprintf (ofd, "%d\t%d\n", sector[i], ti[i]);
	}
	
	fclose (ofd);
}

void write_nonstatic_file_TRACE1(char* outfile)
{
	FILE* ofd = fopen(outfile, "w");

	il = 0;
	for (int i=0; i<l; i++)
	{
		if ( regions[i] == 1 )
			il++;
	}

	fprintf (ofd, "%s\n%d\n%d\n\n", "TRACE1", m, il);

	for (int i=0; i<l; i++)
	{
		if ( regions[i] == 1 )
			fprintf (ofd, "%d\n", sector[i]);
	}

	fclose(ofd);
}