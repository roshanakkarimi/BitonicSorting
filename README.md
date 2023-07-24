
# Bitonic Sorting

Bitonic sorting is a parallel sorting algorithm. This algorithm is suitable for hardware implementation since the order of comparisons is predefined and does not depend on the data.

A bitonic sequence is a cyclic shift of an up-down sequence. The Bitonic Merging algorithm can sort any bitonic sequence in time log2(n), using n/2 comparator-exchangers. At each step, the comparators compare pairs of elements and swap them so that the larger element is always on the top and the smaller on the bottom.



## Bitonic Sorting Steps 

1 - Converting the random input sequence into a bitonic sequence.

2 - Applying bitonic merging to the bitonic sequence to get the sorted sequence

## The algorithm in details

The smallest bitonic sequence is the one of length 2, which is a sequence of only two numbers, which can be either in increasing or decreasing order. To build a bitonic sequence of length 4, we can sort the first two elements of the given sequence in ascending order and the other two elements in descending order, obtaining a four-element up-down sequence. A Bitonic Merger of size 2 (1 time step and only one comparator-exchanger) may be used to do the sorting. We need one merger to sort the elements in ascending order and another in descending. Similarly, we can obtain a bitonic sequence of size 8 by taking two 4-element bitonic sequences and sorting one in ascending order and the other in descending order. Again, we may use of BM of size 4 to sort each of the sequences. The same process can be repeated to obtain sequences of size 16, 32 and so on. 


