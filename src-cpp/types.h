#ifndef TYPES_H_
#define TYPES_H_

#pragma once

#include <vector>
#include <map>
#include <string>
#include <marisa.h>
#include <tbb/tbb.h>
#include "tbb/scalable_allocator.h"

typedef unsigned int uint32;
typedef unsigned long long int uint64;
typedef unsigned char uint8;

typedef uint64 hash_t;
typedef uint32 minhash_t;
typedef uint32 seq_t;

typedef std::vector<uint32> VectorU32;
typedef std::vector<uint8> VectorU8;
typedef std::vector<bool> VectorBool;
typedef std::vector<hash_t> VectorHash;
typedef std::vector<minhash_t> VectorMinHash;
typedef std::vector<seq_t, tbb::scalable_allocator<seq_t>> VectorSeqPos;
typedef std::map<uint32, seq_t> MapKmerCounts;


#endif /*TYPES_H_*/
