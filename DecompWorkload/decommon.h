
#ifndef _DECOMMON_H_
#define _DECOMMON_H_

#include <fstream>
#include <string>
#include <iostream>
#include <queue>
using namespace std;

extern int m;
extern int l;
extern int il;

extern int* sector;
extern int* ti;
extern int* indices;
extern int* flag;
extern int* regions;

extern int range;

// how the sectors are accessed
extern queue<int>* sectori;

// the number of deletes for workload 1 and workload 2
extern int wl1_dels;
extern int wl2_dels;

extern int* dela;


void clean_up();


// compare function for sorting using ti
bool ti_compare (int i,int j);

// is the op in workload 1
bool isWL1 (int i);

// get the IT (invalidation time) from TI (time to invalidation)
int get_it (int i);


// reads the trace file which is the TRACE1 format
void read_trace_file_TRACE1 (char* infile);

// Process the static sectors and mark their region label
void process_static_sectors();

// Calculate the TIs of the sectors that are marked in region 1
void calculate_TIs();

// Decompose the workload with regions marked 1 to 1 and 2 where the region 1 is IRR. 
// Can assume that the TI values are already calculated among the 1 values
void decompose_workload();

// The sub-functions for the decompose
void decompose_build_sector_map();

// Write the non-static data to the tracefile
void write_nonstatic_file_TRACE1(char* outfile);

// Output the contents of the sector array into a tracefile and 
void write_trace_file_TRACE3 (char* outfile);

// Writes out the TRACE2TI file. This is done before the decomposition to compare it
// with the output to make sure everything is the same
void write_trace_file_TRACE2TI (char* outfile);

#endif