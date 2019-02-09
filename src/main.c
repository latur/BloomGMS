#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define num unsigned long long int

#include "_reader.c"
#include "_mumur.c"
#include "_filter.c"

int main(int argc, char *argv[])
{
    unsigned read_length = 100;
    if (argc > 2) read_length = atoi(argv[2]);

    unsigned quality = 12;
    if (argc > 3) quality = atoi(argv[3]);

    Genome * seq = genome_read(argv[1], read_length);
    Filter * bloom = bloom_init(seq->reads, quality);

    for (num i = 0; i < seq->length - read_length; i++) {
        bloom_insert(bloom, seq, i);
    }

    /* --------------------------------------------------------------------- */

    num blocks = (num)( ceil(seq->reads/8) );
    unsigned char * gms = (unsigned char *) malloc(sizeof(unsigned char) * blocks);
    memset(gms, 0, sizeof(unsigned char) * blocks);

    for (num k = 0; k != seq->reads; ++k) {
        // printf("%i", bloom_exist(bloom, seq, k) ? 0 : 1);
        if (!bloom_exist(bloom, seq, k)) setbit(gms, k);
    }
    
    // В фильтре обратный порядок бит. Нужно развернуть
    // Генри Уоррен  "Алгоритмические трюки для программистов"
    for (char v = 0; v != blocks; ++v) {
        gms[v] = (gms[v] & 0x55) << 1 | (gms[v] & 0xAA) >> 1;
        gms[v] = (gms[v] & 0x33) << 2 | (gms[v] & 0xCC) >> 2;
        gms[v] = (gms[v] & 0x0F) << 4 | (gms[v] & 0xF0) >> 4;
    }

    FILE * pFile = fopen ("./track.bin", "wb");
    fwrite (gms , sizeof(unsigned char), blocks, pFile);
    fclose (pFile);

    free(gms);

    /* --------------------------------------------------------------------- */

    genome_clear(seq);
    bloom_clear(bloom);
    return 0;
}

/*
 Build:
 gcc ./src/main.c -o ./dist/exec

 valgrind --leak-check=full --show-leak-kinds=all \
  ./dist/exec ./example.fa 15
 */
