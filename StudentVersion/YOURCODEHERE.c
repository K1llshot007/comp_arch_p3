#include "YOURCODEHERE.h"

typedef unsigned long long i64;

unsigned int lg2pow2(uint64_t pow2){ // helper function for integer lg2; using (double) version from math is not safe
  unsigned int retval=0;
  while(pow2 != 1 && retval < 64/* -- should actually check the local VA bits, but, as seen below, if !64, will exit anyway...*/){
    pow2 = pow2 >> 1;
    ++retval;
  }
  return retval;
}

void  setSizesOffsetsAndMaskFields(cache* acache, unsigned int size, unsigned int assoc, unsigned int blocksize){
  unsigned int localVAbits=8*sizeof(uint64_t*);
  if (localVAbits!=64){
    fprintf(stderr,"Running non-portable code on unsupported platform, terminating. Please use designated machines.\n");
    exit(-1);
  }

  acache->numways=assoc; // sets associativity
  acache->blocksize=blocksize; // sets blocksize
  acache->numsets=size/(blocksize*assoc); // computes total number of sets
  acache->BO = lg2pow2(blocksize); // computes block offset
  acache->TO = acache->BO + lg2pow2(acache->numsets); // computes tag offset
  acache->VAImask= acache->numsets - 1; // computes AND mask for index
  acache->VATmask=(uint64_t)(((uint64_t)1)<<(localVAbits - acache->TO)) - (uint64_t)1; // computes AND mask for tag
}


unsigned long long getindex(cache* acache, unsigned long long address){
  return (address>>acache->BO) & (uint64_t)acache->VAImask; //Returns index bits, masking upper bits
}

unsigned long long gettag(cache* acache, unsigned long long address){
  return (address>>acache->TO) & (uint64_t)acache->VATmask; //Returns tag bits, masking upper bits
}

void writeback(cache* acache, unsigned int index, unsigned int oldestway){
  unsigned long long tag = acache->sets[index].blocks[oldestway].tag;
  unsigned int blockOffset = acache->BO;
  unsigned long long indexSize = lg2pow2(acache->numsets);
  unsigned int blockSize = acache->blocksize;
  uint64_t baseaddress = 0;
  baseaddress = (tag<<(indexSize + blockOffset)) |  (index << blockOffset) ;
  for (unsigned int i =0; i<(blockSize/sizeof(unsigned long long));++i){
    unsigned long long wordsInLine = acache->sets[index].blocks[oldestway].datawords[i];
    unsigned long long whereToWrite = baseaddress + (i * sizeof(unsigned long long));
    cache *cacheToWrite = acache->nextcache;
    // performaccess(cacheToWrite, whereToWrite, 8, 1, wordsInLine, 0);
    StoreWord(cacheToWrite, whereToWrite, wordsInLine);

  }
}

void fill(cache * acache, unsigned int index, unsigned int oldestway, unsigned long long address){
  cache* cacheToFillFrom = acache->nextcache;
  i64 addressToWriteTo = address;
  
  
  uint64_t baseaddress = address & ~((i64)((acache->blocksize)-1));
  for (i64 x=0; x<(acache->blocksize); x+=(sizeof(i64))){
    i64 fromWhereToGetData = baseaddress + x;
    LoadWord(cacheToFillFrom, fromWhereToGetData);
    
  }
}
