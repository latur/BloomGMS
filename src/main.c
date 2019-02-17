#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>

#define BYTE 8
#define num unsigned long long int

#include "_reader.c"
#include "_filter.c"

int main(int argc, char *argv[])
{
    unsigned read_length = 100;
    if (argc > 2) read_length = atoi(argv[2]);

    unsigned quality = 5;
    if (argc > 3) quality = atoi(argv[3]);

    Genome * seq = genome_read(argv[1], read_length);
    Filter * bloom = bloom_init(seq->reads, quality);

    for (num i = 0; i < seq->length - read_length; i++) {
        bloom_insert(bloom, seq, i);
    }

    free(seq->sequence);
    free(bloom->first);

    /* --------------------------------------------------------------------- */

    num blocks = (num)( ceil(seq->reads/8) );
    unsigned char * gms = (unsigned char *) malloc(sizeof(unsigned char) * blocks);
    unsigned char current_byte = 0;
    num bytes_counter = 0;

    for (num k = 0; k != seq->reads; ++k) {
        // if (bloom_exist(bloom, seq, k) > 0) printf("0"); else printf("1");
        current_byte += pow(2,7-k%8) * (bloom_exist(bloom, seq, k) > 0 ? 0 : 1);
        if (k%8 == 7) {
            *(gms + bytes_counter) = current_byte;
            current_byte = 0;
            bytes_counter++;
        }
    }
    if (seq->reads%8 != 0)
        *(gms + bytes_counter) = current_byte;

    FILE * pFile = fopen ("./track.bin", "wb");
    fwrite (gms , sizeof(unsigned char), blocks, pFile);
    fclose (pFile);

    free(gms);

    /* --------------------------------------------------------------------- */

    free(seq);

    free(bloom->hashes);
    free(bloom->second);
    free(bloom);

    return 0;
}

/*
 Build:
 gcc ./src/main.c -o ./dist/exec

 valgrind --leak-check=full --show-leak-kinds=all \
  ./dist/exec ./example.fa 15
 */
