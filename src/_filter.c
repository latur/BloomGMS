typedef struct {
    num length;
    char * second;
    char * first;
    unsigned hash_count;
    num * hashes;
} Filter;

typedef struct {
	char * key;
	unsigned len;
	unsigned seed;
	num hash;
} ThreadData;

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
	/*
	unsigned threads = bloom->hash_count;

    pthread_t * thread = (pthread_t*) malloc(threads * sizeof(pthread_t));
	ThreadData * parts = (ThreadData*) malloc(threads * sizeof(ThreadData));

    for (unsigned t = 0; t < threads; t++){
        parts[t].key = &seq->sequence[i];
        parts[t].len = seq->read_length;
		parts[t].seed = t;
        parts[t].hash = 0;
        pthread_create( &(thread[t]), NULL, hasher, &parts[t]);
    }

    for (unsigned t = 0; t < threads; t++){
        pthread_join(thread[t], NULL);
		bloom->hashes[t] = parts[t].hash % (bloom->length * BYTE);
    }

    free(parts);
    free(thread);
	*/

	// MurmurHash64A
    const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;

    const uint64_t * data = (const uint64_t *) &seq->sequence[i];
    const uint64_t * end = data + (seq->read_length/8);

    for (unsigned seed = 0; seed < bloom->hash_count; seed++) {
	    bloom->hashes[seed] = (uint64_t) seed ^ (seq->read_length * m);
	}

    while(data != end)
    {
        uint64_t k = *data++;
        k *= m;
        k ^= k >> r;
        k *= m;

	    for (unsigned seed = 0; seed < bloom->hash_count; seed++) {
		    bloom->hashes[seed] ^= k;
		    bloom->hashes[seed] *= m;
		}
    }

    const unsigned char * data2 = (const unsigned char*) data;

    for (unsigned seed = 0; seed < bloom->hash_count; seed++) {
	    switch(seq->read_length & 7)
	    {
	        case 7: bloom->hashes[seed] ^= (uint64_t) data2[6] << 48;
	        case 6: bloom->hashes[seed] ^= (uint64_t) data2[5] << 40;
	        case 5: bloom->hashes[seed] ^= (uint64_t) data2[4] << 32;
	        case 4: bloom->hashes[seed] ^= (uint64_t) data2[3] << 24;
	        case 3: bloom->hashes[seed] ^= (uint64_t) data2[2] << 16;
	        case 2: bloom->hashes[seed] ^= (uint64_t) data2[1] << 8;
	        case 1: bloom->hashes[seed] ^= (uint64_t) data2[0];
	        bloom->hashes[seed] *= m;
	    };

	    bloom->hashes[seed] ^= bloom->hashes[seed] >> r;
	    bloom->hashes[seed] *= m;
	    bloom->hashes[seed] ^= bloom->hashes[seed] >> r;
		bloom->hashes[seed] = bloom->hashes[seed] % (bloom->length * BYTE); 
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
