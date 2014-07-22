
#include "common.h"

// no. of sectors
int m;
// length of the workload
int l;
int il;

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

void read_trace_file (char* infile)
{
	// the input file object to read from
	ifstream inputfile (infile);
	if ( !inputfile.is_open() )
	{
		cout << "Inputfile '" << infile << "' could not be opened." << endl;
		exit(-1);
	}

	// Read the header
	string header;
	inputfile >> header;
	inputfile >> m >> l;
	cout << "Inputfile: ";
	cout << "Format: " << header << ", m=" << m << ", l=" << l << endl;

	// Make sure we have the right file format
	if (header != "TRACE1TI")
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
		inputfile >> sector[i];
		inputfile >> ti[i];
		if ( inputfile.eof() )
		{
			cout << "ERROR! Workload length given does not match file" << endl;
			exit(-1);
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
}

void read_trace_file_c (char* infile)
{
	// the input file object to read from
	FILE* inputfile;
	fopen_s (&inputfile, infile, "r");

	// Read the header
	string header;
	fscanf(inputfile, "%s\n%d\n%d\n\n", header.c_str(), &m, &l);
	cout << "Inputfile: ";
	cout << "Format: " << header << ", m=" << m << ", l=" << l << endl;

	// Make sure we have the right file format
	if (header != "TRACE1TI")
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
		fscanf (inputfile, "%d\t%d\n", sector+i, ti+i);
		if ( feof (inputfile)  )
		{
			cout << "ERROR! Workload length given does not match file" << endl;
			exit(-1);
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

	fclose(inputfile);
}

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

// is the op in workload 1
bool isWL1 (int i)
{
	return indices[i] > -1;
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
		op_count [ sector[i] ]++;
		op_last  [ sector[i] ] = i;
	}
	cout << "done" << endl;

	// --------------------------------------------------------------------
	// ---- Process the static sectors
	// --------------------------------------------------------------------
	int statics = 0;
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
		if ( op_count[sector[i]] == 1)
			regions[i] = -1;
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

void decompose_build_sector_map()
{
	// We already have the sector map. 
	// We are just building the indices in the loop
	int cur_index = 0;
	for (int i=0; i<l; i++)
	{
		if ( regions[i] == 1 )
		{
			if ( ti[i] > range )
			{
				indices[i] = -1;
				regions[i] = 2;
			}
		}
	}
}

void decompose_workload()
{
	wl1_dels = 0;
	wl2_dels = 0;
	il = 0;

	//// Make the map of the input file
	cout << "Creating map ... ";
	decompose_build_sector_map();
	cout << "done" << endl;

	//// Figure out how many in wl1 and how many in wl2
	cout << "Processing workload ...";
	//process_dels();
	cout << "done" << endl;
	cout << "WL1_dels: " << wl1_dels << "\tWL2_dels: " << wl2_dels << endl;

	//// Write out the output files
	cout << "Writing output files ...";
	//write_trace_file (outfile1, outfile2);
	cout << "done" << endl;
}

void write_trace_file_TRACE3 (char* outfile)
{
	FILE* ofd = fopen (outfile, "w");

	fprintf (ofd, "%s\n%d\n%d\n\n", "TRACE1", m, l);
	
	for (int i=0; i<l; i++)
	{
		if ( regions[i] == 1 )
			fprintf (ofd, "%d\t%d\n", sector[i], ti[i]);
	}
	
	fclose (ofd);
}