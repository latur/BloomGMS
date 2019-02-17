typedef struct {
    num length;
    char * second;
    char * first;

    unsigned hash_count;
    num * hashes;
} Filter;

uint64_t MurmurHash64A (const void * key, int len, unsigned int seed )
{
    const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;
    
    uint64_t h = seed ^ (len * m);
    
    const uint64_t * data = (const uint64_t *)key;
    const uint64_t * end = data + (len/8);
    
    while(data != end)
    {
        uint64_t k = *data++;
        
        k *= m;
        k ^= k >> r;
        k *= m;
        
        h ^= k;
        h *= m;
    }
    
    const unsigned char * data2 = (const unsigned char*)data;
    
    switch(len & 7)
    {
        case 7: h ^= (uint64_t) data2[6] << 48;
        case 6: h ^= (uint64_t) data2[5] << 40;
        case 5: h ^= (uint64_t) data2[4] << 32;
        case 4: h ^= (uint64_t) data2[3] << 24;
        case 3: h ^= (uint64_t) data2[2] << 16;
        case 2: h ^= (uint64_t) data2[1] << 8;
        case 1: h ^= (uint64_t) data2[0];
            h *= m;
    };
    
    h ^= h >> r;
    h *= m;
    h ^= h >> r;
    
    return h;
}

void setbit(void * field, num index)
{
    char * byte = NULL;
    byte = (char *) field + index / BYTE;
    *byte = *byte | (1 << (index % BYTE));
}

unsigned checkbit(const void * field, num index)
{
    char * byte = NULL;
    byte = (char *) field + index / BYTE;
    return (*byte >> (index % BYTE)) & 1;
}

void make_hashes(Filter * bloom, const Genome * seq, const num i)
{
    unsigned h = 0;
    for (h = 0; h < bloom->hash_count; h++) {
        bloom->hashes[h] = MurmurHash64A(&seq->sequence[i], seq->read_length, h) % (bloom->length * BYTE);
    }
}

unsigned bloom_exist(Filter * bloom, const Genome * seq, const num i)
{
    make_hashes(bloom, seq, i);

    unsigned h = 0;
    unsigned exist = 1;
    for (h = 0; h < bloom->hash_count; h++) {
        if (!checkbit(bloom->second, bloom->hashes[h])) exist = 0;
    }
    
    return exist;
}

void bloom_insert(Filter * bloom, Genome * seq, num i)
{
    make_hashes(bloom, seq, i);

    unsigned h = 0;
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


Filter * bloom_init(num genome_size, unsigned quality)
{
    Filter * bloom = (Filter *) malloc(sizeof(Filter));

    bloom->hash_count = round(2.77259 * quality); // Log[2] * 4 = 2.77259
    bloom->hashes = (num *) malloc(sizeof(num) * bloom->hash_count);

    bloom->length = genome_size * quality;
    bloom->first = malloc(bloom->length);
    bloom->second = malloc(bloom->length);

    memset(bloom->first, 0, bloom->length);
    memset(bloom->second, 0, bloom->length);
    return bloom;
}
