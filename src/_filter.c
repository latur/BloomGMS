typedef struct {
    unsigned hash_count;
    num * hashes;

    char * second;
    char * first;
    num length;
} Filter;

void setbit(void * field, num index)
{
    char * byte = NULL;
    byte = (char *) field + index / __CHAR_BIT__;
    *byte = *byte | (1 << (index % __CHAR_BIT__));
}

unsigned checkbit(const void * field, num index)
{
    char * byte = NULL;
    byte = (char *) field + index / __CHAR_BIT__;
    return (*byte >> (index % __CHAR_BIT__)) & 1;
}

unsigned bloom_exist(Filter * bloom, Genome * seq, num i)
{
    unsigned h = 0;
    
    for (h = 0; h < bloom->hash_count; h++) {
        bloom->hashes[h] = murmur(&seq->sequence[i], seq->read_length, h);
        bloom->hashes[h] *= murmur(&seq->sequence[i], seq->read_length, h + 1024);
        bloom->hashes[h] = bloom->hashes[h] % (bloom->length * __CHAR_BIT__);
    }

    unsigned exist = 1;
    for (h = 0; h < bloom->hash_count; h++) {
        if (!checkbit(bloom->second, bloom->hashes[h])) exist = 0;
    }
    
    return exist;
}

void bloom_insert(Filter * bloom, Genome * seq, num i)
{
    unsigned h = 0;

    for (h = 0; h < bloom->hash_count; h++) {
        bloom->hashes[h] = murmur(&seq->sequence[i], seq->read_length, h);
        bloom->hashes[h] *= murmur(&seq->sequence[i], seq->read_length, h + 1024);
        bloom->hashes[h] = bloom->hashes[h] % (bloom->length * __CHAR_BIT__);
    }

    unsigned exist = 1;
    for (h = 0; h < bloom->hash_count; h++) {
        if (!checkbit(bloom->first, bloom->hashes[h])) exist = 0;
        setbit(bloom->first, bloom->hashes[h]);
    }

    if (exist) {
        for (h = 0; h < bloom->hash_count; h++) {
            setbit(bloom->second, bloom->hashes[h]);
        }
    }
}

Filter * bloom_init(num reads, unsigned quality)
{
    Filter * bloom = (Filter *) malloc(sizeof(Filter));

    // bits = quality * reads * 2
    // optimal = Log[2] * bits / reads = Log[2] * 2 * quality = 1.38629 * quality
    bloom->hash_count = round(1.38629 * quality);
    bloom->hashes = (num *) malloc(sizeof(num) * bloom->hash_count);
    
    bloom->length = reads * quality;

    bloom->first = malloc(bloom->length);
    memset(bloom->first, 0, bloom->length);
    
    bloom->second = malloc(bloom->length);
    memset(bloom->second, 0, bloom->length);

    return bloom;
}

void bloom_clear(Filter * bloom)
{
    free(bloom->first);
    free(bloom->second);
    free(bloom->hashes);
    free(bloom);
}
