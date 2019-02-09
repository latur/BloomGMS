typedef struct {
    unsigned read_length;
    char * sequence;
    num length;
    num reads;
} Genome;

char complement(char ch)
{
    if (ch == 'A') return 'T';
    if (ch == 'T') return 'A';
    if (ch == 'G') return 'C';
    if (ch == 'C') return 'G';
    return ch;
}

Genome * genome_read(char * src, unsigned read_length)
{
    Genome * seq = NULL;

    FILE * f = fopen(src, "r");
    if (f == NULL) {
        printf("Genome file not found\n");
        exit(EXIT_FAILURE);
    }
    fseek(f, 0, SEEK_END);
    num bytes = (num) ftell(f);

    seq = (Genome *) malloc(sizeof(Genome));
    seq->sequence = (char *) malloc(sizeof(char) * bytes * 2);
    seq->read_length = read_length;

    fseek(f, 0, SEEK_SET);

    char ch;
    char state = 'N';
    num i = 0;

    while ((ch = fgetc(f)) != EOF) {
        if (state == 'N' && ch == '>') {
            state = 'T'; // is title
        } else if (ch == '\n') {
            state = 'N'; // is newline
        } else if (state != 'T') {
            seq->sequence[i++] = (ch >= 'a' ? ch - 32 : ch); // to upper case
        }
    }
    fclose(f);
    
    num r = i;
    while (i > 0) seq->sequence[r++] = complement(seq->sequence[--i]);
    
    seq->sequence[r] = '\0';
    seq->length = r;
    seq->reads = seq->length/2 - read_length + 1;

    return seq;
}

void genome_clear(Genome * seq)
{
    free(seq->sequence);
    free(seq);
}
