optwa
=====

by Mochan Shrestha

Tools for offline optimal workload decompositions

Tools
-----
- CacheFilter : removes the operations that would be filtered out by the cache. Takes cache size parameter
- StaticSplit : splits the workload into sub-workloads where all the static sectors are separated
- CalcTI : Calculates the TI (Time to Invalidation) and outputs the TRACE2TI file format
- SplitWLLog : Does the optimal decomposition split

- DecompWorkload : Does what the four tools do in one step and marks workloads into regions. -1. static 0. cache 1. IRR 2. non-IRR remaining and outputs a TRACE3 file

- TRACE3Check : Tool to check that the DecompWorkload did all the steps correctly and the final TRACE3 file is consistent

