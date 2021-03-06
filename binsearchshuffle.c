﻿/*
Shuffled Binary Search In-Place Shuffle

Carl-Henrik Skårstedt (#Sakrac)
This is a reference implementation, something to dig up if I come up with a
need for a binary search.

The purpose of this implementation is to shuffle a sorted array in-place such
that binary search can be performed by starting with the first element and
only looking forward in memory and has closer locality between each step.

Functions

- void ShuffleSortedArray(int *array, int count)
	- shuffles a sorted array of integers
- int ShuffledBinarySearch(int value, int *shuffled_array, int count)
	- finds a value in a shuffled sorted array
- int DeshuffleIndex(int index, int count)
	- converts a shuffled index into a linear index

Background

Binary search is great for finding a value in a large sorted array, but cache
performance suffer from jumping between midpoint to midpoint (start at size/2,
then skip to size/4 or 3*size/4 and so on).

Reasons to shuffle

A simple improvement is to reorganize the sorted array so the array begins
with the middle value, followed by all values lower arranged in the same way
and then all values higher arranged in the same way.

This means that if a match was not found in the current iteration, if the value
is lower (~50% chance) it will be the next value in the array. If the value is
higher the next value in the array will be at the midpoint plus one.

Implementation

The shuffled binary search array can be generated by first sorting values in
one array and then moving those values into another array, however this
implementation shuffles the array in-place.

For larger blocks in the array the operation is:
Move all elements in the lower half of the array down and put the pushed out
value first, then recurse into the lower half and the upper half.

For smaller blocks it is trivial to just swap a few elements around:
- 0 => 0 (1 element)
- 01 => 10 (2 elements)
- 012 => 102 (3 elements)
- 0123 => 2103 (4)
- 01234 => 21043 (5)
- 012345 => 310254 (6)
- 0123456 => 3102546 (7)
- 01234567 => 42103657 (8, starts to get more complicated)
- 012345678 => 421037658
- 0123456789 => 5210438769




count => operations:
- 0 or 1 => do nothing
- 2 => swap (1,0)
- 3 => swap (1,0) (same as 2)
- 4 => swap (2,0)
- 5 => swap (2,0) swap(3,4) (same as 4 + swap(3,4))
- 6 => shift (3,0,2) swap (4,5)
- 7 => shift (3,0,2) swap (4,5) (same as 6)
- 8 => shift (4,0,3) swap(1,2) swap(5,6)
- 9 => shift (4,0,3) swap(1,2) swap(5,7)
- 10 => shift(5,0,3) swap(1,2) swap(6,8)

deshuffle operations:
- 0-5: Same
- 6,7: shift (2,0,3) swap (4,5)
- higher: tmp=first, shift lower half back in memory, put first in middle, subdivide

Given the shuffled binary search array, the search function is trivial to
implement.

- Call ShuffleSortedArray with a previously sorted array to shuffle it
- Call ShuffledBinarySearch with a value to find and the shuffled array
	to find the index (returns -1 if value was not found)

Drawbacks

Insertion and deletion which is trivial with a sorted array becomes more
difficult, to the point that going back to a sorted array and, perform
the operation and then shuffle the array again is a good option.

Key/Value lookup

Reorganizing the array for cache performance only helps if multiple values
fit within a cache line so if array values are keys mapping to values the keys
should reside in one array and the values in another with matching indices for
each key/value.

The returned array index will refer to the location in the shuffled array so
one option is to apply the same shuffle to both the key and the values arrays.
This is fine in most cases but the values could be more costly or awkward to
move around than the keys.

Another option is to _unshuffle_ the index from the key lookup into a linear
index, which is a small O(log n) loop, without any memory access.

- Call DeshuffleIndex to convert a shuffled index into a linear index.

Removal and Insertion

Just for completion and the rare case that a shuffled binary search array with
insertion and removal would actually make sense, here's some code to handle
that.

- void SortShuffledArray(int *array, int count)
	Sorts a shuffled array.
- int RemoveShuffledArrayValue(int value, int *shuffled_array, int count)
	Removes a value from a shuffled array, returns new count.
- int InsertShuffledArrayValue(int value, int *shuffled_array, int count)
	Inserts a value into a shuffled array, checks for duplicate, returns
	new count.

Initially the idea was to just use qsort or something on the shuffled array but
the difference in performance between shuffling an array and sorting it is just
too large.

Keep in mind that calling InsertShuffledArrayValue requires that there is room
for the array to grow. Check the return value from Remove and Insert since it
is valid that the count does not change (Removing a value that doesn't exist or
Inserting a duplicate value would result in 'count' not changing).

Test code

There is a bit of trivial test code that creates randomized arrays, sorts and
shuffles to verify that values can be found in the correct locations.

A note on size

If the typical case is small enough that all values in the array fits into a
cacheline there probably is nothing measurable to gain from a binary search,
or even a shuffled binary search.
*/

#include <string.h>
#include "binsearchshuffle.h"

#define MAX_SHUFFLE_COUNT_LOG2 64
void ShuffleSortedArray(int *array, int count)
{
	// each halfing splits the array in two, but only the upper half needs to go on the stack
	// the lower half is the next step of iteration
	struct { int first, count; } aStack[MAX_SHUFFLE_COUNT_LOG2];
	int stk = 0;

//	assert(count<(1<<MAX_SHUFFLE_COUNT_LOG2)); // ints can represent numbers this big

	int first = 0;		// current section of the array
	int tmp;			// temporary value for swapping elements

	while (count>1 || stk) {
		if (count<=1) {	// count 1 does not need to be shuffled
			stk--;
			first = aStack[stk].first;
			count = aStack[stk].count;
		}

		switch (count) {
			case 2:
			case 3:
				// 2 = > swap(1, 0)	3 = > swap(1, 0) (same as 2)
				tmp = array[first];
				array[first] = array[first+1];
				array[first+1] = tmp;
				count = 0;
				break;
			case 4:
				tmp = array[first];
				array[first] = array[first+2];
				array[first+2] = tmp;
				count = 0;
				break;
			case 5:
				// 5 = > swap(2, 0) swap(3, 4) (same as 4 + swap(3, 4))
				tmp = array[first];
				array[first] = array[first+2];
				array[first+2] = tmp;
				tmp = array[first+3];
				array[first+3] = array[first+4];
				array[first+4] = tmp;
				count = 0;
				break;
			case 6:
			case 7:
				// 6 = > swap(3, 0, 2) swap(4, 5); 7 = > swap(3, 0, 2) swap(4, 5) (same as 6)
				tmp = array[first];
				array[first] = array[first+3];
				array[first+3] = array[first+2];
				array[first+2] = tmp;
				tmp = array[first+4];
				array[first+4] = array[first+5];
				array[first+5] = tmp;
				count = 0;
				break;
			case 8:
				// 8 = > shift(4, 0, 3) swap(1, 2) swap(5, 6)
				tmp = array[first];
				array[first] = array[first+4];
				array[first+4] = array[first+3];
				array[first+3] = tmp;
				tmp = array[first+1];
				array[first+1] = array[first+2];
				array[first+2] = tmp;
				tmp = array[first+5];
				array[first+5] = array[first+6];
				array[first+6] = tmp;
				count = 0;
				break;
			default:
				// count > 8, rotate right first half elements, push second half of elements on the stack
				tmp = array[first+count/2];
				memmove(&array[first+1], &array[first], sizeof(array[0]) * (count/2));
				array[first] = tmp;
				first++;
				aStack[stk].first = first+count/2;
				aStack[stk].count = (count-1)/2;	// count>=8 => (count-1)/2>=3 so no need to check if count<=1
				stk++;
				count = count/2;
				break;
		}
	}
}

int ShuffledBinarySearch(int value, int *shuffled_array, int count)
{
	int index = 0;
	while (count) {
		int read = shuffled_array[index];
		if (value==read)
			return index;		// index into shuffled array where value is found
		else if (value>read) {
			index += count/2+1;
			count = (count-1)/2;
		} else {
			index++;
			count /= 2;
		}
	}
	return -1;	// index not found
}

int DeshuffleIndex(int index, int count)
{
	// check valid range (-1 should be handled since shuffled search returns that to indicate value not found)
	if (index<0 || index>=count)
		return -1;

	int m = count/2;			// midpoint in current block
	int d = m;					// deshuffled index
	while (index) {				// index==0 means d is the deshuffled index and it is done
		if (index>m) {			// next block is upper half
			index -= m+1;		// skip current and skip to middle
			count = (count-1)/2; // size of the upper half
			d++;				// skip current
		} else {				// next block is lower half
			index--;			// skip to next
			count = m;			// size of lower half
			d -= m;				// first deshuffled index
		}
		m = count/2;			// midpoint of the current block
		d += m;					// this is the first value in block
	}
	return d;
}

// Reverse ShuffleSortedArray
void SortShuffledArray(int *array, int count)
{
	struct { int first, count; } aStack[MAX_SHUFFLE_COUNT_LOG2];
	int stk = 0;

	//	assert(count<(1<<MAX_SHUFFLE_COUNT_LOG2)); // ints can represent numbers this big

	int first = 0;		// current section of the array
	int tmp;			// temporary value for swapping elements

	while (count>1 || stk) {
		if (count<=1) {	// count 1 does not need to be shuffled
			stk--;
			first = aStack[stk].first;
			count = aStack[stk].count;
		}

		switch (count) {
			case 2:
			case 3:
				// 2 = > swap(1, 0)	3 = > swap(1, 0) (same as 2)
				tmp = array[first];
				array[first] = array[first+1];
				array[first+1] = tmp;
				count = 0;
				break;
			case 4:
				tmp = array[first];
				array[first] = array[first+2];
				array[first+2] = tmp;
				count = 0;
				break;
			case 5:
				// 5 = > swap(2, 0) swap(3, 4) (same as 4 + swap(3, 4))
				tmp = array[first];
				array[first] = array[first+2];
				array[first+2] = tmp;
				tmp = array[first+3];
				array[first+3] = array[first+4];
				array[first+4] = tmp;
				count = 0;
				break;
			case 6:
			case 7:
				// 6, 7: shift(2, 0, 3) swap(4, 5)
				tmp = array[first];
				array[first] = array[first+2];
				array[first+2] = array[first+3];
				array[first+3] = tmp;
				tmp = array[first+4];
				array[first+4] = array[first+5];
				array[first+5] = tmp;
				count = 0;
				break;
			default:
				// count >= 8, rotate left first half elements, push second half of elements on the stack
				tmp = array[first];
				memmove(&array[first], &array[first+1], sizeof(array[0]) * (count/2));
				array[first+count/2] = tmp;
				first;
				aStack[stk].first = first+1+count/2;
				aStack[stk].count = (count-1)/2;	// count>=8 => (count-1)/2>=3 so no need to check if count<=1
				stk++;
				count = count/2;
				break;
		}
	}
}

// Removes a value from a shuffled array by first detecting the value,
// if found then unshuffle, shift the array in memory and reshuffle.
// Returns new array count.
int RemoveShuffledArrayValue(int value, int *shuffled_array, int count)
{
	int index = ShuffledBinarySearch(value, shuffled_array, count);
	if (index>=0) {
		SortShuffledArray(shuffled_array, count);
		int deshuf = DeshuffleIndex(index, count);
		if (deshuf<(count-1))
			memmove(shuffled_array+deshuf, shuffled_array+deshuf+1, (count-1-deshuf) * sizeof(int));
		count--;
		ShuffleSortedArray(shuffled_array, count);
	}
	return count;
}

// Inserts a value into a shuffled array by first determining if the
// value exists, if not unshuffle, find a slot, make room, assign slot,
// increase array count and reshuffle. Caller is responsible for making
// sure there is room for one more int in the array. Returns new count.
int InsertShuffledArrayValue(int value, int *shuffled_array, int count)
{
	// check for duplicate
	int found = ShuffledBinarySearch(value, shuffled_array, count);
	if (found<0) {
		SortShuffledArray(shuffled_array, count);	// make array linear
		int first = 0;
		int end = count;
		int index = 0;	// index = slot of insertion
		while (end!=first) {
			index = (first+end)/2;
			int read = shuffled_array[index];
			if (read>value && (!index || shuffled_array[index-1]<value))
				break;	// found an insertion slot
			else if (value>read) {
				first = index+1;
			} else {
				end = index;
			}
		}
		if (end==first)
			index = count; // did not find a slot => greatest value
		else
			memmove(shuffled_array+index+1, shuffled_array+index, (count-index) * sizeof(int));
		shuffled_array[index] = value;
		count++;
		ShuffleSortedArray(shuffled_array, count);
	}
	return count;
}

int RegularBinarySearch(int value, int *sorted_array, int end)
{
    int first = 0;
    while (end!=first) {
        int index = (first+end)/2;
        int read = sorted_array[index];
        if (value==read)
            return index;
        else if (value>read) {
            first = index+1;
        } else {
            end = index;
        }
    }
    return -1;	// index not found
}



