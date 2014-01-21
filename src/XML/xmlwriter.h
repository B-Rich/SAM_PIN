#ifndef XMLWRITER_H
#define XMLWRITER_H

#include <iostream>
#include <fstream>
#include "datastructs.h"

class XMLWriter
{
public:
	XMLWriter(void);
	XMLWriter(const char *filename);
	~XMLWriter(void);

	void create_file(const char *filename);
	void write_request(request *rq);

	bool valid(void);
private:
	std::ofstream datafile;
};

#endif