#ifndef __BINSHUFFLE_H__
#define __BINSHUFFLE_H__

void ShuffleSortedArray(int *array, int count);
int ShuffledBinarySearch(int value, int *shuffled_array, int count);
int DeshuffleIndex(int index, int count);

#endif