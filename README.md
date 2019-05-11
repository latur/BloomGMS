# MakeGMS

For a given **genome** and **read size**, the `makegms` module creates a binary track, where 0 corresponds to the index of the repeating read in the genome, and 1 to the index of the unique read. A reed is considered repetitive if it is found more than once in the genome (including reverse complementary).

For example, let the genome be the string `AAAAAATGCA` and the length of the read is `4`

```
gms(AAAAAATGCA, 4) = 0001100

AAAAAATGCA -> AAAAAATGCA + TGCATTTTTT
AAAAAATGCATGCATTTTTT
AAAA                 x 3 -> 0
 AAAA                x 3 -> 0
  AAAA               x 3 -> 0
   AAAT              x 1 -> 1
    AATG             x 1 -> 1
     ATGC            x 2 -> 0
      TGCA           x 2 -> 0
```

## Installation:

```bash
pip3 install --user git+https://github.com/latur/MakeGMS
```

## Usage examples:

To run with the default parameters, you only need to specify the size of the read. In this case, the optimal track building algorithm will be selected, depending on the genome size:

```python
>>> import makegms
>>> track = makegms.run('example.fa', read=15)
>>> ('').join(map(str, track))
'00111111111111000000000111111111111100111111...'
```

To indicate that you want to use QSort based algorithm exactly, pass the number of **threads** as an argument. It makes sense when `threads > 1`. However, if you specify here a number greater than the number of processors you have, performance will decrease:

```python
>>> track = makegms.run('example.fa', read=15, threads=2)
```

To use the algorithm on the filters, specify the quality of the hash function. [The quality](#quality-parameter) is related to the probability of a collision and the amount of RAM consumed:

```python
>>> track = makegms.run('example.fa', read=15, quality=4)
```

## Algorithm Description:

### QSort-based method:

1. Sorting reads. Used QuickSort
2. Сomparison of current and next read (In the sort order)
 - If the reads match, then increase the value of the repeat counter.
 - If the reads do not match, save counter value for previous read and reset the counter for current read

Multithreading is provided by dividing all reads into disjoint blocks using the hash function:  
`block_number = Hash(read) % blocks_count`

### Bloom-based method:

1. Creating two Bloom-filters of the same size: `F1`, `F2`
2. The first round of all reads in the genome:
 - For each read in the genome — add it to filter `F1` and if the read already exists in filter `F1` then add it to filter `F2`
3. The second round of all reads in the genome:
 - Form a binary sequence: each read that is in filter `F2` corresponds to `0`, otherwise `1`

Filter size is determined based on the quality parameter. 

### Quality parameter

To work, you need to allocate memory for two Bloom filters. The number of hash functions of the filter is determined in the optimal way to reduce the likelihood of a collision. Below is a table of memory requirements and the corresponding probability of a false definition of a unique read as repeated. `GS` is Genome Size in bytes

| Quality | Memory  | Hashes | False Positive |
| ------- | ------- | ------ | -------------- |
| 3       | 5 × GS  | 8      | 3.1423 * 10^-3 |
| 4       | 6 × GS  | 11     | 4.5869 * 10^-4 |
| 5       | 7 × GS  | 14     | 6.7134 * 10^-5 |
| 6       | 8 × GS  | 17     | 9.8393 * 10^-6 |
| 7       | 9 × GS  | 19     | 1.4400 * 10^-6 |
| 8       | 10 × GS | 22     | 2.1044 * 10^-7 |
