#include "WorkloadSplitter.h"

int m;
int l;
int il;

int* sector;
int* ti;
int* indices;
int* flag;

int* dela;

queue<int>* sectori;


FILE* tfd;
int tio_count;

WorkloadSplitter::WorkloadSplitter(char* filename)
{
	sector	= NULL;
	ti		= NULL;
	indices	= NULL;
	flag	= NULL;

	dela = new int[l];

	load (filename);

	wl1_dels = 0;
	wl2_dels = 0;

	fopen_s (&tfd, "deltati.txt", "w");
	tio_count = 0;
}


WorkloadSplitter::~WorkloadSplitter(void)
{
	if (sector)		delete[] sector;
	if (ti)			delete[] ti;
	if (indices)	delete[] indices;
	if (flag)		delete[] flag;

	if (dela)		delete[] dela;
	if (sectori)	delete[] sectori;
}

void WorkloadSplitter::load (char* infile)
{
	// the input file object to read from
	FILE* inputfile;
	fopen_s (&inputfile, infile, "r");

	// Read the header
	char header[256];
	fscanf (inputfile, "%s\n%d\n%d\n\n", header, &m, &l);
	cout << "Inputfile: " << infile << endl;
	cout << '\t' << "Format: " << header << ", m=" << m << ", l=" << l << endl;

	// Make sure we have the right file format
	string header_s(header);
	if (header_s != "TRACE1TI")
	{
		cout << "File format is not TRACE1TI!" << endl;
		exit(-1);
	}

	// Read the file into memory
	cout << "Reading file to memory ... ";

	sector = new int[l];
	ti = new int[l];
	for (int i=0; i<l; i++)
	{
		if ( feof (inputfile)  )
		{
			cout << "ERROR! Workload length given does not match file" << endl;
			cout << "i = " << i << endl;
			cout << "l = " << l << endl;
			cout << sector[i] << " " << ti[i] << endl;
			exit(-1);
		}
		fscanf (inputfile, "%d\t%d\n", sector+i, ti+i);
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

	fclose(inputfile);
}

void WorkloadSplitter::get_candidates()
{
	// The candidates will have
	// - TI above the range
	// - not marked for workload 2

	rv_can_count = 0;

	for (int i=0; i<l; i++)
	{
		if ( isWL1(i) && ti[i] >= range )
		{
			rv_can[rv_can_count] = i;
			rv_can_count++;
		}
	}
}

bool ti_compare (int i,int j) 
{
	return (ti[i]>ti[j]); 
}

bool compare (int i, int j)
{
	return indices[i]<indices[j];
}

void WorkloadSplitter::refine_candidates()
{
	// First sort the candidates by their TI
	std::sort (rv_can, rv_can + rv_can_count, ti_compare);

	// Make a list of ones that we can safely remove
	rv_end = 0;
	//int temp_range = range + rv_can_count;
	int temp_range = range;

	while ( rv_end+1 < rv_can_count && ti[rv_can[rv_end+1]] > temp_range )
	{
		rv_end++;
		temp_range++;
	}

	if ( rv_can_count > 0 && rv_end == 0 )
		rv_end = 1;

	int factor = 2;
	if ( rv_end > factor )
		rv_end /= factor;

	//rv_end = rv_can_count;
}

bool WorkloadSplitter::isWL1 (int op) 
{ 
	return indices[op] >= 0; 
}

bool WorkloadSplitter::isWL2 (int op) 
{ 
	return indices[op] < 0; 
}

void WorkloadSplitter::setWL2 ( int op )
{
	indices [op] = -1;
}

int WorkloadSplitter::get_it (int i)
{
	return indices[i] + ti[i];
}

void WorkloadSplitter::remove_op (int op)
{
	// precursors

	int index = indices[op];
	int op_sector = sector[op];

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
			int it = get_it(i);
			if ( it > index )
				ti[i]--;
		}
	}

	//-----------------------------------------------------------------------
	//----------- Decrement the indices after op removal --------------------
	//-----------------------------------------------------------------------

	// Loop over the operations after op
	for (int i=op+1; i<l; i++)
	{
		if ( isWL1(i) )
		{
			// we removed an item from workload1 and so every index after it is changed
			indices[i]--;

			if ( get_it(i) == il )
				ti[i]--;
		}
	}

	indices[op] = -1;
	il--;
}

void WorkloadSplitter::remove_ops()
{
	// ---------------------------------------------------------------------
	// We will be removing all the operations in rv_can to rv_end
	// ---------------------------------------------------------------------
	if ( rv_end == 0 )
		return;

	// sort the operations by their indices
	std::sort (rv_can, rv_can + rv_end, compare);

	int indices_dec = 0;

	// Go through the workload
	for (int i=0; i<l; i++)
	{
		if ( isWL1(i) )
		{
			// Mark the operation as workload 2
			if ( indices_dec < rv_end && i == rv_can[indices_dec] )
			{
				setWL2 ( rv_can[indices_dec] );
				indices_dec++;
				continue;
			}


			// Update TIs - find how many between i and ti at i
			int in_between = 0; 
			int in_btwn_idx = indices_dec;
			
			int it = indices[i] + ti[i];

			if ( indices_dec == rv_end)
			{
			}
			else
			{
				if ( it == il )
				{
					ti[i] -= rv_end;
				}
				else
				{
					if ( indices_dec < rv_end && indices[rv_can[indices_dec]] >= it )
					{
					}
					else
					{
						while ( in_btwn_idx + 1 < rv_end && indices[rv_can[in_btwn_idx+1]] < it )
						{
							in_btwn_idx++;
						}
						in_between = in_btwn_idx - indices_dec + 1;
						ti[i] -= in_between;
					}
				}
			}

			// Update the indices
			while ( indices_dec + 1 < rv_end && i > rv_can[indices_dec+1] )
			{
				indices_dec++;
			}
			// Update indices
			indices[i] -= indices_dec;
		}
	}

	// Subtract the number of operations that we are going to remove
	il -= rv_end;
}

void WorkloadSplitter::write_output_files (char* outfile1, char* outfile2)
{
	FILE* of1;
	fopen_s (&of1, outfile1, "w+");
	FILE* of2;
	fopen_s (&of2, outfile2, "w+");

	fprintf (of1, "%s\n%d\n%d\n%d\n\n", "TRACE2TI", m, il+wl1_dels, il);
	fprintf (of2, "%s\n%d\n%d\n%d\n\n", "TRACE2", m, (l-il)+wl2_dels, l-il);

	for (int i=0; i<l; i++)
	{	
		if ( isWL1(i) )
		{
			//Workload1

			fprintf (of1, "%d\t%d\t%d\n", 1, sector[i], ti[i]);
			//fprintf (of1, "%d\t%d\t%d\t%d\n", 1, indices[i], sector[i], ti[i]);

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

void WorkloadSplitter::process_dels ()
{
	dela = new int[l];
	memset (dela, 0, sizeof(int)*l);
	il = 0;
	
	for (int i=0; i<l; i++)
	{
		int c_sector = sector[i];
		queue<int>* cqueue = &(sectori[c_sector]);

		if ( isWL1(i) )
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
				if ( isWL2(prev_op) )
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
				if ( isWL1(prev_op) )
				{
					wl1_dels++;
					dela[i] = 2;
				}
				cqueue->pop();
			}
		}
	}
}

void WorkloadSplitter::build_sector_map()
{
	sectori = new queue<int>[m];
	for (int i=0; i<l; i++)
	{
		sectori[ sector[i] ].push(i);
	}
}

int WorkloadSplitter::split_down (int range, char* outfile1, char* outfile2)
{
	cout << endl;
	cout << "Starting Split ... " << endl;

	cout << '\t' << "Split point: " << range << endl;
	this->range = range;

	// The indices into the sector array that tells us the candidates we can remove
	rv_can = new int[l];

	//FILE* fd = fopen ("rv.log", "w+");
	while (1)
	{
		// Get the list of candidate points we can remove
		cout << "\t" << "Getting Candidates ...";
		get_candidates();
		cout << " ... done. " << rv_can_count << " found." << endl;

		// Refine the candidate points so that we can remove them safely
		cout << "\t" << "Refining Candidates ...";
		refine_candidates();
		cout << " ... done. " << rv_end << " selected. " << endl;

		if ( rv_end == 0 )
			break;

		//for (int i=0; i<rv_end; i++)
		//{
		//	int op = rv_can[i];
		//	fprintf (fd, "%d\t%d\n", op, ti[op]);
		//}

		// Remove the operations
		cout << "\t" << "Removing operations from W to W2 ...";
		/*for (int j=0; j<rv_end; j++)
		{
			remove_op(rv_can[j]);
		}*/
		remove_ops();
		cout << " ... done. " << endl;
	}
	//fclose(fd);

	cout << "... done" << endl;

	// Clean up
	delete[] rv_can;

	// Make the map of the input file
	cout << "Creating map ... ";
	build_sector_map();
	cout << "done" << endl;

	// Figure out where the deletes need to be
	cout << endl << "Processing deletes ... " << endl;
	process_dels();
	cout << "... done" << endl << endl;

	// Once the workload has been split, write it to file
	cout << "Writing file to disk ... ";
	write_output_files ( outfile1, outfile2 );
	cout << " ... done" << endl;

	return 0;
}

int WorkloadSplitter::find_max_op()
{
	int max = -1;
	int max_i = -1;

	for (int i=0; i<l; i++)
	{
		if ( isWL1(i) && ti[i] > max )
		{
			max = ti[i];
			max_i = i;
		}
	}

	return max_i;
}

int WorkloadSplitter::split_l2 (int range, char* outfile1, char* outfile2)
{
	cout << endl;
	cout << "Starting Split ... " << endl;

	cout << '\t' << "Split point: " << range << endl;
	this->range = range;


	// loop until all the operations under over range are removed starting from the largest
	cout << "Splitting ... ";
	//FILE* fd = fopen ("rv.log", "w+");
	while (1)
	{
		int op = find_max_op();
		if ( ti[op] < range)
			break;
		remove_op(op);

		//fprintf (fd, "%d\t%d\n", op, ti[op]);
	}
	cout << " ... done" << endl;
	//fclose(fd);


	// Make the map of the input file
	cout << "Creating map ... ";
	build_sector_map();
	cout << "done" << endl;

	// Figure out where the deletes need to be
	cout << endl << "Processing deletes ... " << endl;
	process_dels();
	cout << "... done" << endl << endl;

	// Once the workload has been split, write it to file
	cout << "Writing file to disk ... ";
	write_output_files ( outfile1, outfile2 );
	cout << " ... done" << endl;

	return 0;
}

int WorkloadSplitter::linear_search_between (int it)
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

int WorkloadSplitter::binary_search_between (int it)
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

int WorkloadSplitter::get_in_between (int op)
{
	// We have to get the number of removed operations between op and op+ti[op]
	int it = op + ti[op];

	if ( it == l )
		return rv_end;

	// If nothing has been selected for W2
	if ( rv_end == 0 )
	{
		return 0;
	}

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

	//return linear_search_between(it);
	return binary_search_between(it);
}

void WorkloadSplitter::remove_ops_up()
{
	rv_end = 0;

	// percentage stuff
	int p_comp = 0;

	// Iterate through the workload from down to up
	for (int i=l-1; i>=0; i--)
	{
		// Update the TIs for the operation
		int tib = get_in_between (i);
		if ( tib > 0 && tib != rv_end && tio_count < 10000  )
		{
			fprintf (tfd, "%d\t%d\n", i, tib);
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
			rv_can[rv_end] = i;
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


int WorkloadSplitter::split (int range, char* outfile1, char* outfile2)
{
	cout << endl;
	cout << "Starting Split ... " << endl;

	cout << '\t' << "Split point: " << range << endl;
	this->range = range;

	// The indices into the sector array that tells us the candidates we can remove
	rv_can = new int[l];

	// Remove all the processing and remove all the operations
	remove_ops_up();

	cout << "... done" << endl;

	// Clean up
	delete[] rv_can;

	// Make the map of the input file
	cout << "Creating map ... ";
	build_sector_map();
	cout << "done" << endl;

	// Figure out where the deletes need to be
	cout << endl << "Processing deletes ... " << endl;
	process_dels();
	cout << "... done" << endl << endl;

	// Once the workload has been split, write it to file
	cout << "Writing file to disk ... ";
	write_output_files ( outfile1, outfile2 );
	cout << " ... done" << endl;

	return 0;
}
