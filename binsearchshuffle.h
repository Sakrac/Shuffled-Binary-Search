#ifndef __BINSHUFFLE_H__
#define __BINSHUFFLE_H__

void ShuffleSortedArray(int *array, int count); // shuffle a sorted array
int ShuffledBinarySearch(int value, int *shuffled_array, int count); // find the index of a value in a shuffled array
int DeshuffleIndex(int index, int count); // convert a shuffled index into a linear index

void SortShuffledArray(int *array, int count); // sort a shuffled array
int RemoveShuffledArrayValue(int value, int *shuffled_array, int count); // returns updated 'count'
int InsertShuffledArrayValue(int value, int *shuffled_array, int count); // returns updated 'count'

// for comparison with a sorted binary search
int RegularBinarySearch(int value, int *sorted_array, int count);

#endif