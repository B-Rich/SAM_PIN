#include <stdlib.h>
#include <string.h>
#include "xmlwriter.h"
#include "datastructs.h"

int main(int argc, char *argv[])
{
	request *rq = new request;
	request *rq2 = new request;
	XMLWriter *writer = new XMLWriter("output.xml");

	if (writer->valid() != 0) {
		std::cerr << "Error opening file!" << std::endl;
		return -1;
	}

	rq->type = 'c';
	rq->data.cpu.speed = 3.5;
	rq->data.cpu.cpu_id = 4770;
	rq->data.cpu.generation = 12;

	writer->write_request(rq);

	rq2->type = 'r';
	rq2->data.ram.speed = 2333;
	rq2->data.ram.type  = strdup("DDR5");
	rq2->data.ram.size = strdup("8GB");

	writer->write_request(rq2);

	if (rq2->type == 'r') {
		free(rq2->data.ram.type);
		free(rq2->data.ram.size);
	}
	delete rq;
	delete rq2;

	return 0;
}