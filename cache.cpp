#include <iostream>
#include  <iomanip>
#include <math.h>

using namespace std;

#define		DBG				1
#define		DRAM_SIZE		(64*1024*1024)
#define		CACHE_SIZE		(64*1024)

enum cacheResType {MISS=0, HIT=1};

/* The following implements a random number generator */
unsigned int m_w = 0xABABAB55;    /* must not be zero, nor 0x464fffff */
unsigned int m_z = 0x05080902;    /* must not be zero, nor 0x9068ffff */
unsigned int rand_()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}

unsigned int memGen1()
{
	static unsigned int addr=0;
	return (addr++)%(DRAM_SIZE);
}

unsigned int memGen2()
{
	static unsigned int addr=0;
	return  rand_()%(24*1024);
}

unsigned int memGen3()
{
	return rand_()%(DRAM_SIZE);
}

unsigned int memGen4()
{
	static unsigned int addr=0;
	return (addr++)%(4*1024);
}

unsigned int memGen5()
{
	static unsigned int addr=0;
	return (addr++)%(1024*64);
}

unsigned int memGen6()
{
	static unsigned int addr=0;
	return (addr+=32)%(64*4*1024);
}

#define LINE_SIZE		64

// Direct Mapped Cache Simulator
cacheResType cacheSimDM(unsigned int addr)
{
    const int numLines = CACHE_SIZE / LINE_SIZE;

    static bool valid[numLines] = {false};
    static unsigned int tags[numLines] = {0};

    unsigned int blockOffsetBits = log2(LINE_SIZE);
    unsigned int index = (addr >> blockOffsetBits) % numLines;
    unsigned int tag = addr >> (blockOffsetBits + (unsigned int)log2(numLines));

    if (valid[index] && tags[index] == tag)
    {
        return HIT;
    }
    else
    {
        valid[index] = true;
        tags[index] = tag;
        return MISS;
    }
}


cacheResType cacheSimFA(unsigned int addr)
{
    const int numLines = CACHE_SIZE / LINE_SIZE;

    struct CacheLine {
        bool valid;
        unsigned int tag;
        int lruCounter; // Lower = more recently used
    };

    static CacheLine cache[numLines] = {0};
    static int globalCounter = 0; // used to figure out LRU age

	unsigned int blockOffsetBits = static_cast<unsigned int>(log2(LINE_SIZE));
    unsigned int tag = addr >> blockOffsetBits;

    // Step 1: look for a hit
    for (int i = 0; i < numLines; i++) {
        if (cache[i].valid && cache[i].tag == tag) {
            cache[i].lruCounter = ++globalCounter; // update LRU on hit
            return HIT;
        }
    }

    // attempt to find an invalid line
    int victimIndex = -1;
    for (int i = 0; i < numLines; i++) {
        if (!cache[i].valid) {
            victimIndex = i;
            break;
        }
    }

    // remove the LRU
    if (victimIndex == -1) {
        int minLRU = cache[0].lruCounter;
        victimIndex = 0;
        for (int i = 1; i < numLines; i++) {
            if (cache[i].lruCounter < minLRU) {
                minLRU = cache[i].lruCounter;
                victimIndex = i;
            }
        }
    }

    cache[victimIndex].valid = true;
    cache[victimIndex].tag = tag;
    cache[victimIndex].lruCounter = ++globalCounter;

    return MISS;
}

const char *msg[2] = {"Miss","Hit"};

#define		NO_OF_Iterations	10000
int main()
{
	unsigned int hit = 0;
	cacheResType r;
	
	unsigned int addr;
	cout << "Direct Mapped Cache Simulator\n";

	for(int inst=0;inst<NO_OF_Iterations;inst++)
	{
		addr = memGen2();
		r = cacheSimFA(addr);
		if(r == HIT) hit++;
		cout <<"0x" << setfill('0') << setw(8) << hex << addr <<" ("<< msg[r] <<")\n";
	}
	cout << "0x" << setfill('0') << setw(8) << hex << addr <<" ("<< msg[r] <<")\n";

	// Before printing ratio, switch back to decimal
	cout << dec << "Hit ratio = " << (100 * hit / NO_OF_Iterations) << endl;
}