
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
using namespace std;

#include <stdlib.h>

#include "process.h"

string header;
int l;
int m;
int il;

int format;

#define FORMAT_TRACE1	1
#define FORMAT_TRACE2	2

#define OP_WRITE		1
#define OP_DEL			2

void main(int argc, char* argv[])
{
	if (argc <= 2)
	{
		cout << "USAGE: <tracefile> <outfile>" << endl;
		return;
	}

	process2 (argv[1], argv[2]);
}

void process2 (char* infile, char* outfile)
{
	ifstream inputfile (infile);

	// ------------------------------------------------------------------------
	// ------------------------- Read the header of the file
	// ------------------------------------------------------------------------
	inputfile >> header;

	// Determine the file format depending on the header
	if ( header == "TRACE1")
	{
		format = FORMAT_TRACE1;
		inputfile >> m >> l;
		cout << header << "\tm=" << m << "\tl=" << l << endl;
	}
	else if (header == "TRACE2")
	{
		format = FORMAT_TRACE2;
		inputfile >> m >> l >> il;
		cout << header << "\tm=" << m << "\tl=" << l << "\til=" << il << endl;
	}
	else
	{
		cout << "File format " << header << " is not known. Exiting ..." << endl;
		exit(-1);
	}

	// Read the file into memory
	int* sector = new int[l];
	int* ti = new int[l];

	int* index;
	int* op;

	if ( format == FORMAT_TRACE2 )
	{
		op = new int[l];
		index = new int[l];
	}

	// ------------------------------------------------------------------------
	// --------------------------- Create the data structures
	// ------------------------------------------------------------------------
	queue<int>* sectori = new queue<int>[m];

	// ------------------------------------------------------------------------
	// ------------------------ Read the file and fill up the data structures
	// ------------------------------------------------------------------------
	cout << "Reading file and filling data structures ... ";
	int cur_index = 0;
	for (int i=0; i<l; i++)
	{
		if ( format == FORMAT_TRACE1 )
		{
			inputfile >> sector[i];
		}

		if ( format == FORMAT_TRACE2 )
		{
			inputfile >> op[i] >> sector[i];
			index[i] = cur_index;
			if ( op[i] == 1 )
			{
				cur_index++;
			}
		}

		if ( inputfile.eof() )
		{
			l = i;
			break;
		}

		sectori[ sector[i] ].push(i);
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
		int c_sector = sector[i];
		queue<int>* cqueue = &(sectori[c_sector]);

		// if the top of the queue is the current sector, remove it
		if ( cqueue->front() == i )
		{
			sectori[c_sector].pop();
		}
		
		if ( format == FORMAT_TRACE1 )
		{
			// If there is nothing in the queue, put the tti as the end of the workload
			if ( cqueue->empty() )
			{
				ti[i] = l - i;
				continue;
			}

			// It's not empty, then set the tti as the next value
			int nexti = cqueue->front();
			ti[i] = nexti - i;
			cqueue->pop();
		}
		else
		{
			if ( op[i] == OP_WRITE )
			{
				// If there is nothing in the queue, put the tti as the end of the workload
				if ( cqueue->empty() )
				{
					ti[i] = il - index[i];
					continue;
				}

				// It's not empty, then set the tti as the next value
				int nexti = cqueue->front();
				ti[i] = index[nexti] - index[i];
				cqueue->pop();
			}
		}

		// output the percentage counter
		int pcn = (int)(i/(double)l*100.0f);
		if ( pcn > pc )
		{
			pc = pcn;
			cout << "Percentage complete: " << pc << endl;
		}
	}
	delete[] sectori;

	cout << "done" << endl;

	// ------------------------------------------------------------------------
	// ---------------------------- Write to the output file
	// ------------------------------------------------------------------------
	cout << "Writing file to disk ... ";
	FILE* fd;
	fopen_s (&fd, outfile, "w+");
	//ofstream outputfile (outfile);

	// header
	if ( format == FORMAT_TRACE1)
	{
		fprintf (fd, "%s\n%d\n%d\n\n", "TRACE1TI", m, l);
	}
	else if ( format == FORMAT_TRACE2)
	{
		fprintf (fd, "%s\n%d\n%d\n%d\n\n", "TRACE2TI", m, l, il);
	}

	// the sector and it's TI
	for (int i=0; i<l; i++)
	{
		if ( format == FORMAT_TRACE1)
		{
			fprintf(fd, "%d\t%d\n", sector[i], ti[i]);
		}
		else if ( format == FORMAT_TRACE2)
		{
			if ( op[i] == OP_WRITE )
			{
				fprintf(fd, "1\t%d\t%d\n", sector[i], ti[i]);
			}
			else
			{
				fprintf(fd, "2\t%d\n", sector[i], ti[i]);
			}
		}
	}

	fclose(fd);

	delete[] sector;
	delete[] ti;

	if ( format == FORMAT_TRACE2 )
	{
		delete[] index;
		delete[] op;
	}

	cout << "done" << endl;

}

void process (char* infile, char* outfile)
{
	ifstream inputfile (infile);

	// Read the header of the file
	string header;
	int l;
	int m;
	inputfile >> header;
	inputfile >> m >> l;
	cout << header << "\tm=" << m << "\tl=" << l << endl;

	// Read the file into memory
	int* sector = new int[l];
	int pc = 0;
	for (int i=0; i<l; i++)
	{
		inputfile >> sector[i];
	}
	// output the few lines to see if the data is being read correctly
	//for (int i=l-5; i<l; i++)
	//{
	//	cout << sector[i] << endl;
	//}

	// Calculate the TIs
	int* ti = new int[l];
	for (int i=0; i<l; i++)
	{
		// the current sector
		int csector = sector[i];

		// find the next time the sector appears again
		bool found = false;
		for (int j=i+1; j<l; j++)
		{
			if ( sector[j] == csector )
			{
				ti[i] = j - i;
				found = true;
				break;
			}
		}

		// coun't find the sector. output -1
		if ( !found )
		{
			ti[i] = l - i;
		}

		// output the percentage counter
		int pcn = (int)(i/(double)l*100.0f);
		if ( pcn > pc )
		{
			pc = pcn;
			cout << "Percentage complete: " << pc << endl;
		}
	}
	inputfile.close();

	// Write to the output file
	ofstream outputfile (outfile);

	// header
	outputfile << "TRACE1TI" << endl;
	outputfile << m << endl;
	outputfile << l << endl;
	outputfile << endl;

	// the sector and it's TI
	for (int i=0; i<l; i++)
	{
		outputfile << sector[i] << '\t' << ti[i] << endl;
	}

	outputfile.close();

	delete[] sector;
	delete[] ti;
}

