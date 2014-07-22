
#ifndef _WORKLOAD_SPLITTER_H_
#define _WORKLOAD_SPLITTER_H_

#define _CRT_SECURE_NO_WARNINGS 1

#include <cstdlib>
#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
using namespace std;

class WorkloadSplitter
{
public:
	WorkloadSplitter(char* filename);
	~WorkloadSplitter(void);

	int split (int range, char* outfile1, char* outfile2);

	// Split algorithm that goes downwards
	int split_down (int range, char* outfile1, char* outfile2);

	// Split using the method by removing one op at a time
	int split_l2 (int range, char* outfile1, char* outfile2);

private:
	void load (char* filename);

	// Get the list of candidates that we can remove. 
	void get_candidates();

	// Refine the candidates so that they can removed without problems
	void refine_candidates();

	void remove_ops();

	// Removes a single operation
	void remove_op( int op );

	// Set the operation belonging to workload 2
	void setWL2 ( int sector );

	// get the IT (invalidation time) from TI (time to invalidation)
	int get_it (int i);

	// Build the sector map
	void build_sector_map();

	// Process the deletes
	void process_dels ();

	// Write out the output files
	void write_output_files (char* outfile1, char* outfile2);

	bool isWL1 (int op);
	bool isWL2 (int op);

	int find_max_op();

	// Removes all the operations going up
	void remove_ops_up();

	// Get the number of removed operations between op and ti[op]
	int get_in_between (int op);

	// Search methods
	int linear_search_between (int it);
	int binary_search_between (int it);

protected:
	int range;

	// For the removal candidates
	int* rv_can;
	int rv_can_count;
	// End of the selection from the candidates
	int rv_end;

	// The number of deletes in the workloads
	int wl1_dels, wl2_dels;
};

#endif