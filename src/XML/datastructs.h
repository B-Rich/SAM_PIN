#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <iostream>
#include <string>
#include <vector>

/* Structs for the different data we need to capture */
struct cpuinfo {
	double speed;
	int cpu_id;
	int generation;
};

struct raminfo {
	double speed;
	char *type;
	char *size;
};

struct opinfo {
	char *name;
	int total;
	std::vector<clock_t> *call_times;
	char *binned_times;
};

struct cacheinfo {
	char *type;
	int loadhits;
	int loadmisses;
	int loadaccess;
	float loadmissrate;
	int storehits;
	int storemisses;
	int storeaccess;
	float storemissrate;
	int totalhit;
	int totalmiss;
	int totalaccess;
	float totalmissrate;
};

// Struct for our instruction calls
struct Instruction
{
    int total = 0;
    std::vector<clock_t> call_times;
};


/* Switch/case the type here to handle each type of data */
struct request {
	union {
		struct cpuinfo cpu;
		struct raminfo ram;
		struct opinfo op;
		struct cacheinfo cache;
	} data;
	unsigned char type;
};

#endif