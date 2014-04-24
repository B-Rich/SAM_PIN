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

typedef UINT64 CACHE_STATS; // type of cache hit/miss counters

#include "pin_cache.H"


ofstream OutFile;
struct Instruction opCount[1200];

clock_t calltime;
clock_t start;
double total_elapsed;

namespace IL1
{
    // 1st level instruction cache: 32 kB, 32 B lines, 32-way associative
    const UINT32 cacheSize = 32*KILO;
    const UINT32 lineSize = 32;
    const UINT32 associativity = 32;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_NO_ALLOCATE;

    const UINT32 max_sets = cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = associativity;

    typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
}
LOCALVAR IL1::CACHE il1("L1 Instruction Cache", IL1::cacheSize, IL1::lineSize, IL1::associativity);

namespace DL1
{
    // 1st level data cache: 32 kB, 32 B lines, 32-way associative
    const UINT32 cacheSize = 32*KILO;
    const UINT32 lineSize = 32;
    const UINT32 associativity = 32;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_NO_ALLOCATE;

    const UINT32 max_sets = cacheSize / (lineSize * associativity);
    const UINT32 max_associativity = associativity;

    typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
}
LOCALVAR DL1::CACHE dl1("L1 Data Cache", DL1::cacheSize, DL1::lineSize, DL1::associativity);

namespace UL2
{
    // 2nd level unified cache: 2 MB, 64 B lines, direct mapped
    const UINT32 cacheSize = 2*MEGA;
    const UINT32 lineSize = 64;
    const UINT32 associativity = 1;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

    const UINT32 max_sets = cacheSize / (lineSize * associativity);

    typedef CACHE_DIRECT_MAPPED(max_sets, allocation) CACHE;
}
LOCALVAR UL2::CACHE ul2("L2 Unified Cache", UL2::cacheSize, UL2::lineSize, UL2::associativity);

namespace UL3
{
    // 3rd level unified cache: 16 MB, 64 B lines, direct mapped
    const UINT32 cacheSize = 16*MEGA;
    const UINT32 lineSize = 64;
    const UINT32 associativity = 1;
    const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

    const UINT32 max_sets = cacheSize / (lineSize * associativity);

    typedef CACHE_DIRECT_MAPPED(max_sets, allocation) CACHE;
}
LOCALVAR UL3::CACHE ul3("L3 Unified Cache", UL3::cacheSize, UL3::lineSize, UL3::associativity);

LOCALFUN VOID Ul2Access(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType)
{
    // second level unified cache
    const BOOL ul2Hit = ul2.Access(addr, size, accessType);

    // third level unified cache
    if ( ! ul2Hit) ul3.Access(addr, size, accessType);
}

LOCALFUN VOID InsRef(ADDRINT addr)
{
    const UINT32 size = 1; // assuming access does not cross cache lines
    const CACHE_BASE::ACCESS_TYPE accessType = CACHE_BASE::ACCESS_TYPE_LOAD;

   

    // first level I-cache
    const BOOL il1Hit = il1.AccessSingleLine(addr, accessType);

    // second level unified Cache
    if ( ! il1Hit) Ul2Access(addr, size, accessType);
}

LOCALFUN VOID MemRefMulti(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType)
{
    // first level D-cache
    const BOOL dl1Hit = dl1.Access(addr, size, accessType);

    // second level unified Cache
    if ( ! dl1Hit) Ul2Access(addr, size, accessType);
}

LOCALFUN VOID MemRefSingle(ADDRINT addr, UINT32 size, CACHE_BASE::ACCESS_TYPE accessType)
{

    // first level D-cache
    const BOOL dl1Hit = dl1.AccessSingleLine(addr, accessType);

    // second level unified Cache
    if ( ! dl1Hit) Ul2Access(addr, size, accessType);
}

LOCALFUN VOID Cache(INS ins, VOID *v)
{
    // all instruction fetches access I-cache
    INS_InsertCall(
        ins, IPOINT_BEFORE, (AFUNPTR)InsRef,
        IARG_INST_PTR,
        IARG_END);

    if (INS_IsMemoryRead(ins))
    {
        const UINT32 size = INS_MemoryReadSize(ins);
        const AFUNPTR countFun = (size <= 4 ? (AFUNPTR) MemRefSingle : (AFUNPTR) MemRefMulti);

        // only predicated-on memory instructions access D-cache
        INS_InsertPredicatedCall(
            ins, IPOINT_BEFORE, countFun,
            IARG_MEMORYREAD_EA,
            IARG_MEMORYREAD_SIZE,
            IARG_UINT32, CACHE_BASE::ACCESS_TYPE_LOAD,
            IARG_END);
    }

    if (INS_IsMemoryWrite(ins))
    {
        const UINT32 size = INS_MemoryWriteSize(ins);
        const AFUNPTR countFun = (size <= 4 ? (AFUNPTR) MemRefSingle : (AFUNPTR) MemRefMulti);

        // only predicated-on memory instructions access D-cache
        INS_InsertPredicatedCall(
            ins, IPOINT_BEFORE, countFun,
            IARG_MEMORYWRITE_EA,
            IARG_MEMORYWRITE_SIZE,
            IARG_UINT32, CACHE_BASE::ACCESS_TYPE_STORE,
            IARG_END);
    }
}

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


VOID ImageLoad(IMG img, void *v)
{
    fstream output;
    output.open("output.xml", std::fstream::out);

    output << "\t<Functions>\n";

    for( SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym) )
    {
        string symPureName =  PIN_UndecorateSymbolName(SYM_Name(sym), UNDECORATION_COMPLETE);
        
            output << "\t\t<Function name=\"" << symPureName << "\">\n" \
            << "\t\t\t<SymIndex>" << SYM_Index(sym) << "</SymIndex>\n" \
            << "\t\t\t<SymAddress>" << SYM_Value(sym) << "</SymAddress>\n" \
            << "\t\t</Function>\n";
    }

    output << "\t</Functions>\n";
    output << endl;

    output.close();
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

    writer->write_tag("\t", "Instruction");
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

    writer->write_tag("\t", "/Instruction");

    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << "Our Count: " << total << endl;
    OutFile.close();

    std::cerr << il1;
    std::cerr << dl1;
    std::cerr << ul2;
    std::cerr << ul3;
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

GLOBALFUN int main(int argc, char * argv[])
{
    PIN_InitSymbols();

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    INS_AddInstrumentFunction(Cache, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    start = clock();

    OutFile << "Start time: " << start << endl;

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
