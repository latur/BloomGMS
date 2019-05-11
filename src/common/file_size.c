num fileSize(char * src)
{
    FILE * f = fopen(src, "r");
	if (f == NULL) {
		printf("File not found: %s\n", src);
		exit(EXIT_FAILURE);
	}
    fseek(f, 0, SEEK_END);
    return (num) ftell(f);
}
