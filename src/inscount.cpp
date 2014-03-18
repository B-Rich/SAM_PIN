/*BEGIN_LEGAL
Intel Open Source License

Copyright (c) 2002-2013 Intel Corporation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <iomanip>
#include <time.h>

#include "XML/datastructs.h"
#include "XML/xmlwriter.h"
#include "xed-interface.h"
#include "pin.H"

ofstream OutFile;
struct Instruction opCount[1200];

clock_t calltime;
clock_t start;
double total_elapsed;

// Holds instruction count for a single procedure
typedef struct RtnCount
{
    string _name;
    string _image;
    ADDRINT _address;
    RTN _rtn;
    UINT64 _rtnCount;
    UINT64 _icount;
    struct RtnCount * _next;
} RTN_COUNT;

// Linked list of instruction counts for each routine
RTN_COUNT * RtnList = 0;

// This function is called before every instruction is executed
VOID docount(int op)
{
    calltime = clock() - start;

    opCount[op].total++;
    opCount[op].call_times.push_back(calltime);
}

VOID docountsimple(UINT64 * counter)
{
    (*counter)++;
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    // Insert a call to docount before every instruction, no arguments are passed
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_UINT32, INS_Opcode(ins), IARG_END);
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "inscount.out", "specify output file name");

const char * StripPath(const char * path)
{
    const char * file = strrchr(path,'/');
    if (file)
        return file+1;
    else
        return path;
}

VOID Routine(RTN rtn, VOID *v)
{
    
    // Allocate a counter for this routine
    RTN_COUNT * rc = new RTN_COUNT;

    // The RTN goes away when the image is unloaded, so save it now
    // because we need it in the fini
    rc->_name = RTN_Name(rtn);
    rc->_image = StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str());
    rc->_address = RTN_Address(rtn);
    rc->_icount = 0;
    rc->_rtnCount = 0;

    // Add to list of routines
    rc->_next = RtnList;
    RtnList = rc;
            
    RTN_Open(rtn);
            
    // Insert a call at the entry point of a routine to increment the call count
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)docountsimple, IARG_PTR, &(rc->_rtnCount), IARG_END);
    
    // For each instruction of the routine
    for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
    {
        // Insert a call to docount to increment the instruction counter for this rtn
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docountsimple, IARG_PTR, &(rc->_icount), IARG_END);
    }

    
    RTN_Close(rtn);
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Get total runtime of application
    total_elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    double bin_amount = total_elapsed / 10;
    double temp;
    long bins[10] = {0};

    long total = 0;
    std::string final = " ";
    std::vector<clock_t>::iterator iter;
    
    request *rq = new request;

    XMLWriter *writer = new XMLWriter("output.xml");
    if (writer->valid() != 0) {
        std::cerr << "Error opening file!" << std::endl;
    }

    writer->write_tag("Instruction");
    // Bin times
    for (int x = 0; x < 1200; ++x) {
        if(opCount[x].total == 0)
            continue;

        // Bin it all
        for (iter = opCount[x].call_times.begin(); iter != opCount[x].call_times.end(); ++iter) {
            temp = (double)(*iter)/CLOCKS_PER_SEC;
            if(temp < bin_amount)
                bins[0]++;
            if(temp >= bin_amount && temp < (bin_amount*2))
                bins[1]++;
            if(temp >= (bin_amount*2) && temp < (bin_amount*3))
                bins[2]++;
            if(temp >= (bin_amount*3) && temp < (bin_amount*4))
                bins[3]++;
            if(temp >= (bin_amount*4) && temp < (bin_amount*5))
                bins[4]++;
            if(temp >= (bin_amount*5) && temp < (bin_amount*6))
                bins[5]++;
            if(temp >= (bin_amount*6) && temp < (bin_amount*7))
                bins[6]++;
            if(temp >= (bin_amount*7) && temp < (bin_amount*8))
                bins[7]++;
            if(temp >= (bin_amount*8) && temp < (bin_amount*9))
                bins[8]++;
            if(temp >= (bin_amount*9) && temp < (bin_amount*10))
                bins[9]++;
        }

        // Convert to string output
        for (int z = 0; z < 10; ++z)
        {
            if(z == 10) {
                final += std::to_string(bins[z]);
            } else {
                final += std::to_string(bins[z]) + " ";
            }
        }

        // Setup the write request for XML writer
        rq->type = 'o';
        rq->data.op.name = strdup(OPCODE_StringShort(x).c_str());
        rq->data.op.total = opCount[x].total;
        rq->data.op.call_times = &opCount[x].call_times;
        rq->data.op.binned_times = strdup(final.c_str());

        writer->write_request(rq);

        // Reset array
        memset(bins, 0, sizeof(bins));
        final = "";
    }

    writer->write_tag("/Instruction");

    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << "Our Count: " << total << endl;
    OutFile.close();

 for (RTN_COUNT * rc = RtnList; rc; rc = rc->_next)
    {
        if (rc->_icount > 0)
            cout  << setw(23) << rc->_name << " "
                  << setw(15) << rc->_image << " "
                  << setw(18) << hex << rc->_address << dec <<" "
                  << setw(12) << rc->_rtnCount << " "
                  << setw(12) << rc->_icount << endl;
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number olsf dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char * argv[])
{

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    /* Segfaults on this... */
    RTN_AddInstrumentFunction(Routine, 0);
    
    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    start = clock();

    OutFile << "Start time: " << start << endl;

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
