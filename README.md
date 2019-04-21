# Bloom GMS-Maker

## Installation:

```bash
pip3 install --user git+https://github.com/latur/BloomGMS
```

## Usage in python:

```python
import bloomgms
track = bloomgms.make('human.fasta', read=100, quality=6)
# Output: track = [0,0,0,0,1,1,1,0,0,0,0,0,1,1,1, ...]
```

`human.fa` — Path to the file with the genome (fasta)  
`read=10` — Read size  
`quality=6` — Constant for [memory usage](#about-the-quality-parameter)


## Compile as an independent application:

```bash
git clone https://github.com/latur/BloomGMS.git
cd BloomGMS && mkdir build
gcc src/main.c -std=c99 -m64 -O3 -o build/bloomgms
```

## Usage:

```bash
# ./build/bloomgms [fasta file] [read length] [quality: 3 - 8]
./build/bloomgms test.fa 5 4
# The result will be saved in -> track.bin
```


## Algorithm Description:

For a given **genome** (size `G` b.p.) and a given **read size** `L`, we make two passes with a sliding window of size `L` with a step of `1`.  
– The first passage through the genome breaks the genome into reads (`G - L + 1` items). Each of these reads is placed in the Bloom Filter.  
– The second passage through the genome with the same sliding window checks the existence of each read in the Bloom Filter more than 2 times. A binary track is formed based on this, where `0` corresponds to repeating reads, and `1` to unique reads.

For example, let the genome be the string `AAAAAATGCA` and the length of the read `4`

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

If there are several chromosomes or scaffolds in a fasta file, they will be combined into one common genome. For example, for the file `example.fa`:

```fasta
>NC_011750.1:1-999
TTccgcccAaGGCGTTTT

>NC_011750.2:1-999
AAAAAAAAAGTCATGGTT

>NC_011750.3:1-999
GCGGAAGGTAAA
```

```python
>>> import bloomgms
>>> track = bloomgms.make('./example.fa', read=5)
>>> ('').join(map(str, track))
'00111111111111000000000111111111111100111111'
```

## About the quality parameter

To work, you need to allocate memory for two Bloom filters. The number of hash functions of the filter is determined in the optimal way to reduce the likelihood of a collision. Below is a table of memory requirements and the corresponding probability of a false definition of a unique read as repeated. `GS` is Genome Size in bytes

| Quality | Memory  | Hashes | False Positive |
| ------- | ------- | ------ | -------------- |
| 3       | 5 × GS  | 8      | 3.1423 * 10^-3 |
| 4       | 6 × GS  | 11     | 4.5869 * 10^-4 |
| 5       | 7 × GS  | 14     | 6.7134 * 10^-5 |
| 6       | 8 × GS  | 17     | 9.8393 * 10^-6 |
| 7       | 9 × GS  | 19     | 1.4400 * 10^-6 |
| 8       | 10 × GS | 22     | 2.1044 * 10^-7 |
