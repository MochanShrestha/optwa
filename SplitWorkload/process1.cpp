
#include "process.h"

bool sec_order (int i, int j)
{
	return i<j;
}

void remove_op (int op)
{
	// Change the indices value

	// Decrease the TI values of all the operations after the removed operation
	for (int i=0; i<op; i++)
	{
		int abs_up = i + ti[i];
		if ( abs_up > op )
		{
			ti[i]--;
		}
	}
}

void process1 (char* infile, char* outfile, char* outfile2, int range)
{
	//-----------------------------------------------------------------------
	//----------- Read the input workload into memory -----------------------
	//-----------------------------------------------------------------------

	read_trace_file (infile);

	//-----------------------------------------------------------------------
	// ---------- Sort the workload by the TI values -----------------------
	//-----------------------------------------------------------------------
	cout << "Splitting workloads ..." << endl;

	// Create the vector that has the indicees to be sorted
	cout << "Sorting ...";

	// Sort the indices
	std::vector<int> ivector (indices, indices+l);
	std::sort (ivector.begin(), ivector.end(), ti_compare);
	cout << " done" << endl;

	//Testing ... output the sorted data
	//for (std::vector<int>::iterator it=ivector.begin(); it!=ivector.begin()+10; ++it)
	//{
	//	int index = *it;
	//	//cout << index << endl;
	//	cout << index << ", " << sector[index] << ", " << ti[index] << endl;
	//}

	//-----------------------------------------------------------------------
	// ---------- Split the Workload ----------------------------------------
	//-----------------------------------------------------------------------
	std::vector<int>::iterator split = ivector.begin();
	int index = *split;
	int remove_ops = 0;

	while (ti[index] > range)
	{
		// Remove the operation from the workload
		remove_op (index);
		remove_ops++;

		// Resort the values below the split point
		if ( remove_ops % 100 == 0 )
		{
			std::sort (split, ivector.end(), ti_compare);
		}

		++split;
		index = *split;
	}

	cout << "... done splitting" << endl;

	//-----------------------------------------------------------------------
	// ---------- Output the split workload ---------------------------------
	//-----------------------------------------------------------------------

	cout << "Writing Split Workload 1 ... ";

	// First sort the operations left in workload1
	std::sort (split, ivector.end(), sec_order);
	//for (std::vector<int>::iterator it=split; it!=split+1000; ++it)
	//{
	//	int index = *it;
	//	cout << index << ", " << sector[index] << ", " << ti[index] << endl;
	//}

	// Write out the output1 file
	ofstream outputfile1(outfile);
	// header
	outputfile1 << "TRACE1TI" << endl;
	outputfile1 << m << endl;
	outputfile1 << (ivector.end() - split) << endl;
	outputfile1 << endl;

	// the meat
	// the sector and it's TI
	for (std::vector<int>::iterator it=split; it!=ivector.end(); ++it)
	{
		int index = *it;
		outputfile1 << sector[index] << '\t' << ti[index] << endl;
	}
	outputfile1.close();
	cout << "done" << endl;


	cout << "Writing Split Workload 1 ... ";

	// Sort the operations in workload2
	std::sort (ivector.begin(), split-1, sec_order);
	//for (std::vector<int>::iterator it=ivector.begin(); it!=ivector.begin() + 20; ++it)
	//{
	//	int index = *it;
	//	cout << index << ", " << sector[index] << ", " << ti[index] << endl;
	//}

	// Write out the output2 file
	ofstream outputfile2(outfile2);
	// header
	outputfile2 << "TRACE1TI" << endl;
	outputfile2 << m << endl;
	outputfile2 << (split - 1 - ivector.begin()) << endl;
	outputfile2 << endl;

	// the meat
	// the sector and it's TI
	for (std::vector<int>::iterator it=ivector.begin(); it!=split-1; ++it)
	{
		int index = *it;
		outputfile2 << sector[index] << '\t' << ti[index] << endl;
	}
	outputfile2.close();

	cout << "done" << endl;

	//-----------------------------------------------------------------------
	//------------ CLean up -------------------------------------------------
	//-----------------------------------------------------------------------
	clean_up();
}