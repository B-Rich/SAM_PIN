#include "xmlwriter.h"

using namespace std;

static void write_cpu(ofstream *output, request *rq)
{
	*output << "<CPU>\n" \
	<< "\t<Speed>" << rq->data.cpu.speed << "</Speed>\n" \
	<< "\t<CPUID>" << rq->data.cpu.cpu_id << "</CPUID>\n" \
	<< "\t<Generation>" << rq->data.cpu.generation << "</Generation>\n" \
	<< "</CPU>" << endl;
}

static void write_ram(ofstream *output, request *rq)
{
	*output << "<RAM>\n" \
	<< "\t<Speed>" << rq->data.ram.speed << "</Speed>\n" \
	<< "\t<Type>" << rq->data.ram.type << "</Type>\n" \
	<< "\t<Size>" << rq->data.ram.size << "</Size>\n" \
	<< "</RAM>" << endl;
}

void XMLWriter::write_tag(const std::string format, const std::string tag)
{
	
	this->datafile << format << "<" << tag << ">" << endl;

}

static void write_instruction(ofstream *output, request *rq)
{
	*output << "\t\t<Instruction name=\"" << rq->data.op.name <<"\" total=\"" \
	<< rq->data.op.total << "\">" << rq->data.op.binned_times;
	
	// here we need to do times in bins


	*output << "</Instruction>" <<endl;
}

XMLWriter::~XMLWriter(void)
{
	this->datafile.close();
}

XMLWriter::XMLWriter(const char *filename)
{
	this->datafile.open(filename, std::fstream::app);

	if (this->datafile.fail()) {
		cerr << "Could not create file: " << filename << endl;
	}
}

bool XMLWriter::valid(void)
{
	return (this->datafile.fail() ? -1 : 0);
}

void XMLWriter::create_file(const char *filename)
{
	this->datafile.open(filename);

	if (this->datafile.fail()) {
		cerr << "Could not create file: " << filename << endl;
	}
}

void XMLWriter::write_request(request *rq)
{
	switch (rq->type) {
	case 'c':
		write_cpu(&this->datafile, rq);
	break;
	case 'r':
		write_ram(&this->datafile, rq);
	break;
	case 'o':
		write_instruction(&this->datafile, rq);
	break;
	default:
		cerr << "Cannot read request type: " << rq->type <<endl;
	break;
	}
}