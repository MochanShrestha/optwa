
#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

#include "common.h"

// uses the STL sort
void process1 (char* infile, char* outfile1, char* outfile2, int range);

// in place finding max algorithm
void process2 (char* infile, char* outfile1, char* outfile2, int range);

// two pass approximation algorithm
void process3 (char* infile, char* outfile1, char* outfile2, int range);

// sorts (doesn't find max), removes and then outputs the workloads
void process4 (char* infile, char* outfile1, char* outfile2, int range);

#endif