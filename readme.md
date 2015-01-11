#Shuffled Binary Search In-Place Shuffle

Carl-Henrik Skårstedt (#Sakrac)
This is a reference implementation, something to dig up if I come up with a need for a binary search.

The purpose of this implementation is to shuffle a sorted array in-place such that binary search can be performed by starting with the first element and only looking forward in memory and has closer locality between each step.

##Functions

- void **ShuffleSortedArray**(int *array, int count)
	- shuffles a sorted array of integers
- int **ShuffledBinarySearch**(int value, int *shuffled_array, int count)
	- finds a value in a shuffled sorted array
- int **DeshuffleIndex**(int index, int count)
	- converts a shuffled index into a linear index

##Background

Binary search is great for finding a value in a large sorted array, but cache performance suffer from jumping between midpoint to midpoint (start at size/2, then skip to size/4 or 3*size/4 and so on).

##Reasons to shuffle

A simple improvement is to reorganize the sorted array so the array begins with the middle value, followed by all values lower arranged in the same way and then all values higher arranged in the same way.

This means that if a match was not found in the current iteration, if the value is lower (~50% chance) it will be the next value in the array. If the value is higher the next value in the array will be at the midpoint plus one.

##Implementation

The shuffled binary search array can be generated by first sorting values in one array and then moving those values into another array, however this implementation shuffles the array in-place.

For larger blocks in the array the operation is:
Move all elements in the lower half of the array down and put the pushed out value first, then recurse into the lower half and the upper half.

For smaller blocks it is trivial to just swap a few elements around:
- 0 => 0 (1 element)
- 01 => 10 (2 elements)
- 012 => 102 (3 elements)
- 0123 => 2103 (4)
- 01234 => 21043 (5)
- 012345 => 310254 (6)
- 0123456 => 3102546 (7)
- 01234567 => 42103657 (8, starts to get more complicated)

count => operations:
- 0 or 1 => do nothing
- 2 => swap (1,0)
- 3 => swap (1,0) (same as 2)
- 4 => swap (2,0)
- 5 => swap (2,0) swap(3,4) (same as 4 + swap(3,4))
- 6 => shift (3,0,2) swap (4,5)
- 7 => shift (3,0,2) swap (4,5) (same as 6)

Given the shuffled binary search array, the search function is trivial to implement.

- Call **ShuffleSortedArray** with a previously sorted array to shuffle it
- Call **ShuffledBinarySearch** with a value to find and the shuffled array to find the index (returns -1 if value was not found)

##Drawbacks

Insertion and deletion which is trivial with a sorted array becomes more difficult, to the point that going back to a sorted array and, perform the operation and then shuffle the array again is a good option.

##Key/Value lookup

Reorganizing the array for cache performance only helps if multiple values fit within a cache line so if array values are keys mapping to values the keys should reside in one array and the values in another with matching indices for each key/value.

The returned array index will refer to the location in the shuffled array so one option is to apply the same shuffle to both the key and the values arrays. This is fine in most cases but the values could be more costly or awkward to move around than the keys.

Another option is to _unshuffle_ the **index** from the key lookup into a **linear index**, which is a small O(log n) loop, without any memory access.

- Call **DeshuffleIndex** to convert a shuffled index into a linear index.

##Test code

There is a bit of trivial test code that creates randomized arrays, sorts and shuffles to verify that values can be found in the correct locations.

##A note on size
 
If the typical case is small enough that all values in the array fits into a cacheline there probably is nothing measurable to gain from a binary search, or even a shuffled binary search.
