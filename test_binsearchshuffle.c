#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "binsearchshuffle.h"

#define MAX_ARRAY_SIZE 1024
static int qsortInts(const void *a, const void *b) { return *(const int*)a - *(const int*)b; }

int TestShuffle()
{
	int values[MAX_ARRAY_SIZE];		// note: the only reason to keep two arrays here is
	int shuffled[MAX_ARRAY_SIZE];	// that it is easier to figure out which value to search for

	int success = 1;

	for (int count = 2; count<MAX_ARRAY_SIZE; count++) {
		for (int i = 0; i<count; i++)
			values[i] = rand();
		qsort(values, count, sizeof(int), qsortInts);

		// two identical values invalidates this search test
		// but it works fine with binary search (it would just
		// never return one of the indices)
		int add = 0;
		for (int i = 0; i<count; i++) {
			int next_add = 0;
			if (values[i]==values[i+1])
				next_add++;
			values[i] += add;
			add += next_add;
		}

		// copy the sorted/unique array and start shuffle it
		memcpy(shuffled, values, count*sizeof(int));
		ShuffleSortedArray(shuffled, count);

		// try to find each value
		for (int i = 0; i<count; i++) {
			int index = ShuffledBinarySearch(values[i], shuffled, count);
			int deshuffled_index = DeshuffleIndex(index, count);
			if (i!=deshuffled_index) {
				success = 0;
				printf("Problem: linear index=%d, shuffled index=%d, deshuffled index=%d\n", i, index, deshuffled_index);
			}
		}
	}
	return success;
}


int main(int argc, char **argv)
{
	srand((unsigned int)time(NULL));
	if (!TestShuffle())
		return 1;
	return 0;
}
