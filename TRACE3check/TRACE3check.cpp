
#include <iostream>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string>

// Compare Function
void compareIRRandTRACE2 (char* irrfile, char* trace3file);

// Write out the region 1 write operations into an output file
void outputRegion1Ops (char* trace3file, char* outfile);
void outputWriteOps (char* irrfile, char* outfile);

// Other utility functions
// Compare the cache output files
void compareCacheOutputFiles();
// Compare the non-static "dynamic" sectors
void compareNonStaticOutputFiles();
// Compare the TI files from the CalcTI and the DecompWorkload for debugging purposes
void compareTIfiles();

void main (int argc, char* argv[])
{
	//compareCacheOutputFiles();
	//compareNonStaticOutputFiles();
	//compareTIfiles();
	//return;

	// irrfile:		the input trace file
	// trace3file:	the output trace file that has the decomposed workload with classes

	if (argc <= 2)
	{
		cout << "USAGE: <irrfile> <TRACE3file>" << endl;
		return;
	}

	cout << "IRRFile: " << argv[1] << endl;
	cout << "TRACE3File: " << argv[2] << endl << endl;


	compareIRRandTRACE2 (argv[1], argv[2]);
	//outputRegion1Ops (argv[2], "trace3ops.dat");
	//outputWriteOps (argv[1], "traceirrwrites.dat");
}


void compareIRRandTRACE2 (char* irrfile, char* trace3file)
{
	// ---------------------------------------------------------------------------------
	// ----------------------- Reading Headers 
	// ---------------------------------------------------------------------------------

	cout << "Reading headers to memory .... " << endl;

	// Read the headers for the TRACE2TI IRR trace file
	FILE* t2ti;
	fopen_s ( &t2ti, irrfile, "r");
	char header[255];
	int m1, l1, il1;
	fscanf_s (t2ti, "%s\n%d\n%d\n%d\n\n", header, 255, &m1, &l1, &il1);
	cout << "IRR TRACE2: ";
	cout << "Format: " << header << ", m=" << m1 << ", l=" << l1 << ", il=" << il1 << endl;

	// Make sure we have the right file format
	if ( strcmp (header, "TRACE2TI") != 0 )
	{
		cout << "File format is not TRACE2TI" << endl << endl;
		exit(-1);
	}

	// Read the headers for the TRACE3 region files
	FILE* trace3;
	fopen_s ( &trace3, trace3file, "r");
	int m2, l2;
	fscanf_s (trace3, "%s\n%d\n%d\n\n", header, 255, &m2, &l2);
	cout << "TRACE3: ";
	cout << "Format: " << header << ", m=" << m2 << ", l=" << l2 << endl;
	
	// Make sure we have the right file format
	if ( strcmp (header, "TRACE3") != 0 )
	{
		cout << "File format is not TRACE3" << endl << endl;
		exit(-1);
	}

	cout << "... Done" << endl << endl;


	// ---------------------------------------------------------------------------------
	// -------------------- Go through the entire workload
	// ---------------------------------------------------------------------------------
	
	cout << "Reading through the workload ... " << endl;

	// Data that is read from the file
	int op1, sector1, ti1;
	int sector2, region2;
	bool success = false;

	for (int i=0; i<l2; i++)
	{
		// Read an operation from the TRACE3 file
		fscanf_s (trace3, "%d\t%d\n", &sector2, &region2);
		if ( feof(trace3) )
		{
			if ( i != l2 - 1 )
			{
				cout << "The workload was too short" << endl;
				cout << "i=" << i << ", l=" << l2 << endl;
				exit(-1);
			}
			else
			{
				if ( feof(t2ti) )
				{
					break;
				}
			}
		}

		// Check that the file is region 1 data
		if ( region2 != 1 )
		{
			// Continue on. Don't read from the rest of the decomposed IRR TRACE2 file
			continue;
		}

		// The operation is region 1. Get the next operation from the file that is a write
		op1 = 0;
		while ( op1 != 1 )
		{
			fscanf_s (t2ti, "%d\t", &op1);
			if ( op1 == 1 )
			{
				// Write operation
				fscanf_s (t2ti, "%d\t%d\n", &sector1, &ti1);
			}
			else
			{
				// Delete operation. op = 2
				fscanf_s (t2ti, "%d\n", &sector1);
			}

			/*if ( feof(t2ti) )
			{
				cout << "TRACE2 IRR workload ran out of operations!" << endl;
				exit(-1);
			}*/
		}

		// Just compare sector1 with sector2 and make sure they are the same
		if ( sector1 != sector2 )
		{
			cout << "i=" << i << ", sector1=" << sector1 << ", sector2=" << sector2 << endl;
			exit(-1);
		}
		else
		{
			//cout << "i=" << i << ", sector1=" << sector1 << ", sector2=" << sector2 << endl;
		}

		/*if ( success ) 
			break;*/
	}

	/*if ( feof (trace3) )
	{
		cout << "Reached end of file" << endl;
	}
	else
	{
		char c;
		cout << "-----------------------------------------" << endl;
        while ( !feof(trace3) ) 
		{
			c = fgetc (trace3);
			cout << c;
        }
		cout << "-----------------------------------------" << endl;
	}*/


	/*for (int i=0; i<l1; i++)
	{
		fscanf_s (t2ti, "%d\t", &op1);
		if ( op1 == 1 )
		{
			// Write operation
			fscanf_s (t2ti, "%d\t%d\n", &sector1, &ti1);
			//if (i < 10)
			//	cout << "1\t" << sector1 << "\t" << ti1 << endl;
		}
		else
		{
			// Delete operation. op = 2
			fscanf_s (t2ti, "%d\n", &sector1);
			//if ( i < 10 )
			//	cout << "2\t" << sector1 << endl;
		}
	}*/

	//if ( feof (t2ti) )
	//{
	//	cout << "Reached end of file" << endl;
	//}
	//else
	//{
	//	char c;

	//	cout << "-----------------------------------------" << endl;
 //       while ( !feof(t2ti) ) 
	//	{
	//		c = fgetc (t2ti);
	//		cout << c;
 //       }
	//	cout << "-----------------------------------------" << endl;
	//}

	cout << "... Done" << endl;
	cout << "SUCCESS! Both have the same workloads!" << endl;
	cout << endl;


	// We are done. Clean up
	fclose(t2ti);
	fclose(trace3);
}

void outputRegion1Ops (char* trace3file, char* outfile)
{
	// Read the headers for the TRACE3 region files
	FILE* trace3;
	char header[255];
	fopen_s ( &trace3, trace3file, "r");
	int m2, l2;
	fscanf_s (trace3, "%s\n%d\n%d\n\n", header, 255, &m2, &l2);
	cout << "TRACE3: ";
	cout << "Format: " << header << ", m=" << m2 << ", l=" << l2 << endl;
	
	// Make sure we have the right file format
	if ( strcmp (header, "TRACE3") != 0 )
	{
		cout << "File format is not TRACE3" << endl << endl;
		exit(-1);
	}

	// Open the output file
	FILE* ofd;
	fopen_s ( &ofd, outfile, "w");

	int sector2, region2;

	for (int i=0; i<l2; i++)
	{
		// Read an operation from the TRACE3 file
		fscanf_s (trace3, "%d\t%d\n", &sector2, &region2);
		if ( feof(trace3) && i != l2 - 1 )
		{
			cout << "The workload was too short" << endl;
			cout << "i=" << i << ", l=" << l2 << endl;
			exit(-1);
		}

		// Check that the file is region 1 data
		if ( region2 != 1 )
		{
			// Continue on. Don't read from the rest of the decomposed IRR TRACE2 file
			continue;
		}

		fprintf (ofd, "%d\n", sector2);
	}
}

void outputWriteOps (char* irrfile, char* outfile)
{
	// Read the headers for the TRACE2TI IRR trace file
	FILE* t2ti;
	fopen_s ( &t2ti, irrfile, "r");
	char header[255];
	int m1, l1, il1;
	fscanf_s (t2ti, "%s\n%d\n%d\n%d\n\n", header, 255, &m1, &l1, &il1);
	cout << "IRR TRACE2: ";
	cout << "Format: " << header << ", m=" << m1 << ", l=" << l1 << ", il=" << il1 << endl;

	// Make sure we have the right file format
	if ( strcmp (header, "TRACE2TI") != 0 )
	{
		cout << "File format is not TRACE2TI" << endl << endl;
		exit(-1);
	}

	// Open the output file
	FILE* ofd;
	fopen_s ( &ofd, outfile, "w");

	// Go through the workload
	int op1, sector1, ti1;

	for (int i=0; i<l1; i++)
	{
		fscanf_s (t2ti, "%d\t", &op1);
		if ( op1 == 1 )
		{
			// Write operation
			fscanf_s (t2ti, "%d\t%d\n", &sector1, &ti1);
			fprintf (ofd, "%d\n", sector1);
		}
		else
		{
			// Delete operation. op = 2
			fscanf_s (t2ti, "%d\n", &sector1);
		}
	}
}

void compareCacheOutputFiles()
{
	FILE* fd1;
	FILE* fd2;

	fopen_s (&fd1, "tracec.dat", "r");
	fopen_s (&fd2, "tracec2.dat", "r");

	char header[255];
	int m1, l1;
	fscanf_s (fd1, "%s\n%d\n%d\n\n", header, 255, &m1, &l1);
	cout << "Format: " << header << ", m=" << m1 << ", l=" << l1 << endl;
	int m2, l2;
	fscanf_s (fd2, "%s\n%d\n%d\n\n", header, 255, &m2, &l2);
	cout << "Format: " << header << ", m=" << m2 << ", l=" << l2 << endl;

	int sector1, sector2;
	for (int i=0; i<l1; i++)
	{
		fscanf_s (fd1, "%d\n", &sector1);
		fscanf_s (fd2, "%d\n", &sector2);
		if ( sector1 != sector2 )
		{
			cout << "i=" << i << ", sector1=" << sector1 << ", sector2=" << sector2 << endl;
			exit(-1);
		}
	}

	cout << "SUCCESS!! Files are identical!" << endl;
}

void compareNonStaticOutputFiles()
{
	FILE* fd1;
	FILE* fd2;

	fopen_s (&fd1, "traced.dat", "r");
	fopen_s (&fd2, "traced2.dat", "r");

	char header[255];
	int m1, l1;
	fscanf_s (fd1, "%s\n%d\n%d\n\n", header, 255, &m1, &l1);
	cout << "Format: " << header << ", m=" << m1 << ", l=" << l1 << endl;
	int m2, l2;
	fscanf_s (fd2, "%s\n%d\n%d\n\n", header, 255, &m2, &l2);
	cout << "Format: " << header << ", m=" << m2 << ", l=" << l2 << endl;

	int sector1, sector2;
	for (int i=0; i<l1; i++)
	{
		fscanf_s (fd1, "%d\n", &sector1);
		fscanf_s (fd2, "%d\n", &sector2);
		if ( sector1 != sector2 )
		{
			cout << "i=" << i << ", sector1=" << sector1 << ", sector2=" << sector2 << endl;
			exit(-1);
		}
	}

	// Make sure the files are both empty
	if (!feof(fd2))
	{
		cout << "fd2 longer than fd1" << endl;
		char c;
		cout << "-----------------------------------------" << endl;
        while ( !feof(fd2) ) 
		{
			c = fgetc (fd2);
			cout << c;
        }
		cout << "-----------------------------------------" << endl;

		cout << "-----------------------------------------" << endl;
        while ( !feof(fd1) ) 
		{
			c = fgetc (fd1);
			cout << c;
        }
		cout << "-----------------------------------------" << endl;
		exit(-1);
	}

	cout << "SUCCESS!! Non-static files are identical!" << endl;
}

void compareTIfiles()
{
	FILE* fd1;
	FILE* fd2;

	fopen_s (&fd1, "traceti.dat", "r");
	fopen_s (&fd2, "traceti2.dat", "r");

	char header[255];
	int m1, l1;
	fscanf_s (fd1, "%s\n%d\n%d\n\n", header, 255, &m1, &l1);
	cout << "Format: " << header << ", m=" << m1 << ", l=" << l1 << endl;
	int m2, l2;
	fscanf_s (fd2, "%s\n%d\n%d\n\n", header, 255, &m2, &l2);
	cout << "Format: " << header << ", m=" << m2 << ", l=" << l2 << endl;

	if ( m1 != m2 || l1 != l2 )
	{
		// This is normal since I did not bother to actually go through and make sure l is the right value
		cout << "m and l do not match" << endl;
	}

	int sector1, ti1;
	int sector2, ti2;
	for (int i=0; i<l1; i++)
	{
		fscanf_s (fd1, "%d\t%d\n", &sector1, &ti1);
		fscanf_s (fd2, "%d\t%d\n", &sector2, &ti2);
		if ( sector1 != sector2 )
		{
			cout << "i=" << i << ", sector1=" << sector1 << ", sector2=" << sector2 << endl;
			exit(-1);
		}
		if ( ti1 != ti2 )
		{
			cout << "TIs do not match" << endl;
			cout << "i=" << i << ", sector1=" << sector1 << ", sector2=" << sector2 << endl;
			cout << "i=" << i << ", ti1=" << ti1 << ", ti2=" << ti2 << endl;
			exit(-1);
		}
	}

	cout << "SUCCESS! The files are identical" << endl;

	fclose (fd1);
	fclose (fd2);
}