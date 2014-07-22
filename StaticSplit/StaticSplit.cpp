

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#include <stdlib.h>

int m;
int l;
int il;

int* sector;
int* indices;

void main(int argc, char* argv[])
{
	if (argc <= 3)
	{
		cout << "USAGE: <tracefile> <out_tracefile> <static>" << endl;
		return;
	}

	// --------------------------------------------------------------------
	// read the tracefile to memory
	// --------------------------------------------------------------------
	// the input file object to read from
	ifstream inputfile (argv[1]);

	// Read the header
	string header;
	inputfile >> header;
	inputfile >> m >> l;
	cout << "Inputfile: ";
	cout << "Format: " << header << ", m=" << m << ", l=" << l << endl;

	// Make sure we have the right file format
	if (header != "TRACE1")
	{
		cout << "File format is not TRACE1TI!" << endl;
		exit(-1);
	}

	// Read the file into memory
	cout << "Reading file to memory ... ";

	sector = new int[l];
	for (int i=0; i<l; i++)
	{
		inputfile >> sector[i];

		if ( inputfile.eof() )
		{
			cout << "ERROR! Workload length given does not match file. Current i=" << i << ", l=" << l << endl;
			cout << "Setting l=" << i << endl;
			l = i;
			break;
		}
	}

	cout << " done" << endl;
	inputfile.close();

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
	// ---- Output the trace files
	// --------------------------------------------------------------------
	FILE* outu = fopen (argv[2], "w+");
	FILE* outs = fopen (argv[3], "w+");
	//ofstream outu(, outs(argv[3]);

	// write out the headers
	// Write out the headers for both the files
	fprintf (outu, "%s\n%d\n%d\n\n", "TRACE1", m, l-statics);
	fprintf (outs, "%s\n%d\n%d\n\n", "TRACE1", m, statics);
	//outu << "TRACE1TI" << endl;
	//outs << "TRACE1TI" << endl;
	//outu << m << endl;
	//outs << m << endl;
	//outu << l - statics << endl;
	//outs << statics << endl;
	//outu << endl;
	//outs << endl;

	for (int i=0; i<l; i++)
	{
		if ( op_count[sector[i]] <= 1)
			fprintf (outs, "%d\n", sector[i]);
			//outs << sector[i] << endl;
		else
			fprintf (outu, "%d\n", sector[i]);
			//outu << sector[i] << endl;
	}

	fclose(outu);
	fclose(outs);

	delete[] op_count;
	delete[] op_last;
}