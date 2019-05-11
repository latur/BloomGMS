char complement(char ch)
{
    if (ch == 'A') return 'T';
    if (ch == 'T') return 'A';
    if (ch == 'G') return 'C';
    if (ch == 'C') return 'G';
    return ch;
}

Genome * reader(char * src)
{
    Genome * seq = NULL;
	
	big file_size = fileSize(src);

    seq = (Genome *) malloc(sizeof(Genome));
    seq->sequence = (char *) malloc(sizeof(char) * file_size * 2);

    char ch;
    char state = 'N';
    big i = 0;

    FILE * f = fopen(src, "r");
    while ((ch = fgetc(f)) != EOF) {
        if (state == 'N' && ch == '>') {
            state = 'T'; // is title
        } else if (ch == '\n') {
            state = 'N'; // is newline
        } else if (state != 'T') {
            seq->sequence[i++] = (ch >= 'a' ? ch - 32 : ch); // To Uppercase
        }
    }
    fclose(f);
    
    big r = i;
    while (i > 0) seq->sequence[r++] = complement(seq->sequence[--i]);

    seq->sequence[r] = '\0';
    seq->length = r;

    return seq;
}
