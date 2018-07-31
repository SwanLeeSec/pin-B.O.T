/*BEGIN_LEGAL 
Intel Open Source License 
Copyright (c) 2002-2012 Intel Corporation. All rights reserved.
 
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

/* Branch Observation Tool 
	Lee SeungHwan		lesh@tistory.com
	*/

#include "pin.H"
#include <iostream>
#include <fstream>
#include <iomanip>

/* ================================================================== */
// Global variables 
/* ================================================================== */

UINT64 brcCount = 0;
UINT64 tCount = 0;
UINT64 ntCount = 0;
UINT64 allBranch = 0;
float exeRatio = 0;
float takenRatio = 0;

INS brcIns;

std::ostream * out = &cerr;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "", "specify file name for B.O.T output");

KNOB<BOOL>   KnobCount(KNOB_MODE_WRITEONCE,  "pintool",
    "count", "1", "count instructions, basic blocks and threads in the application");


/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*!
 *  Print out help message.
 */
INT32 Usage()
{
    cerr << "This tool prints out the number of dynamically executed " << endl <<
            "branches in the application." << endl << endl;

    cerr << KNOB_BASE::StringKnobSummary() << endl;

    return -1;
}


static VOID AtBranch(ADDRINT ip, ADDRINT target, BOOL taken)
{
	if(INS_IsBranch(brcIns) && INS_HasFallThrough(brcIns))
    {
		brcCount++;
	
		printf("Addr: %p \n", ip);					
		printf("Target: %p \n", target);
		*out << "Asm: " << INS_Disassemble(brcIns);	
		if(taken)
		{
			*out << "[Taken]" << endl;
			tCount++;
		}
		else
		{
			*out << "[Not Taken]" << endl;
			ntCount++;
		}
	}
	   
}


VOID Instruction(INS ins, VOID *v)
{	
	if(INS_IsBranch(ins) && INS_HasFallThrough(ins))
	{				
		brcIns = ins;
		allBranch++;
       INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)AtBranch, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
    		
	}
}


VOID Fini(INT32 code, VOID *v)
{
	exeRatio =  (float) brcCount/ (float) allBranch;
	takenRatio = (float) tCount/ (float) brcCount;
	*out << endl << endl;
	*out <<	 "============ Branch Observation Tool ==========" << endl;
	*out <<	 "============================ Lee Seung Hwan ===" << endl;
	*out <<	 "============================= lesh@tistory.com " << endl << endl;
    *out <<  "===============================================" << endl;
    *out <<  "B.O.T analysis results: " << endl;
	*out <<  "Number of all branches: " << allBranch  << endl;	
	*out <<  "Executed branches: " << brcCount << endl;
	printf("Execute ratio (Executed / All): %f\n",exeRatio);
	printf("Taken ratio (Taken / Executed): %f\n",takenRatio);
	*out <<	 "Taken: " << tCount << "	Not taken: " << ntCount << endl;
    *out <<  "===============================================" << endl;
}


int main(int argc, char *argv[])
{
    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid 
    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }
    
    string fileName = KnobOutputFile.Value();

    if (!fileName.empty()) { out = new std::ofstream(fileName.c_str());}

    if (KnobCount)
    {
		
		// my
		INS_AddInstrumentFunction(Instruction,0);

        // Register function to be called when the application exits
        PIN_AddFiniFunction(Fini, 0);

		
    }
    
    cerr <<  "===============================================" << endl;
    cerr <<  "This application is instrumented by B.O.T" << endl;
    if (!KnobOutputFile.Value().empty()) 
    {
        cerr << "See file " << KnobOutputFile.Value() << " for analysis results" << endl;
    }
    cerr <<  "===============================================" << endl;

    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
