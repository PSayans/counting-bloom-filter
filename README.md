# counting-bloom-filter

The following project consists on the optimization of pollution algorithms for Bloom Filters, more specifically the attack described in P. Reviriego and O. Rottenstreich, “Pollution Attacks on Counting Bloom Filters for Black Box Adversaries” 16th International Conference on Network and Service Management (CNSM), 2020.

## Counting Bloom filters

An implementation of Counting Bloom Filters (CBF) can be found in the project. The filter can be set up with different configurations for size and number of hash functions. 
It also allows using any desired hash function, since the filter only receives a pointer to the function.

## Lookup attack

The lookup attack is implemented with different posible parametrizations:

Expected arguments:
1: Length of vector F.
2: Length of vector T.
3: Number of rounds.
4: Size of the Bloom filter.
5: Number of hashes.
6: Output file for the results.
7: Type of progression for T (linear or fixed).
8: Factor of progression when linear mode selected (just type 0 when fixed legth used).

## Prediction attacks

Two variants of the original lookup attack are also included. One with fixed number of lookup rounds and another one with fixed FPP. For more information about the details of the implementation please read the paper.

## Results

All the obtained results for the project can be found in the repository. For more information about the analysis of the results please read the paper.
