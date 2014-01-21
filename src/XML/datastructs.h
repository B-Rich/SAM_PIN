#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <iostream>
#include <string>

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

/* Switch/case the type here to handle each type of data */
struct request {
	union {
		struct cpuinfo cpu;
		struct raminfo ram;
	} data;
	unsigned char type;
};

#endif