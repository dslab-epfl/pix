#include "pin.H"
#include <iostream>
#include <fstream>
#include <string>

// This is more or less copied from the pintool examples

ofstream trace;
static bool is_counting = false;
static UINT64 instr_count = 0;
static UINT64 mem_count = 0;

typedef struct {
  unsigned long ip;
  std::string function;
  std::string assembly;
} instruction_data_t;

std::vector<std::pair<bool, unsigned long>> addresses;

VOID log_read_op(VOID *ip, VOID *addr) {
 if(is_counting)  addresses.push_back(std::make_pair(0, (unsigned long)addr));
}

VOID log_write_op(VOID *ip, VOID *addr) {
  if(is_counting) addresses.push_back(std::make_pair(1, (unsigned long)addr));
}

VOID log_instruction(instruction_data_t *id) {
  if (is_counting) {

  trace << std::hex << std::uppercase << id->ip << " |";

  trace  << id->function << " | " << id->assembly << " |";

  for (auto a : addresses) {
    trace << " " << (a.first ? "w" : "r") << a.second;
  }
  addresses.clear();

  trace << std::endl;
  }
}

VOID doinstrcount()
{
	if (is_counting) {
		instr_count++;
	}
}

VOID domemcount(VOID* ip, VOID* addr)
{
	if (is_counting) {
		mem_count++;
	}
}


VOID trace_before(CHAR* name, ADDRINT size)
{
	is_counting = true;
}

VOID trace_instr(INS ins, VOID* v)
{

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)doinstrcount, IARG_END);

    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
    // prefixed instructions appear as predicated instructions in Pin.
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp) || INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)domemcount,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_END);
	    break;
        }
    }

    instruction_data_t *id = new instruction_data_t();
    id->ip = INS_Address(ins);
    id->function = RTN_FindNameByAddress(id->ip);
    id->assembly = INS_Disassemble(ins);

    for (UINT32 memOp = 0; memOp < memOperands; memOp++) {
    if (INS_MemoryOperandIsRead(ins, memOp)) {
      INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)log_read_op,
                               IARG_INST_PTR, IARG_MEMORYOP_EA, memOp,
                               IARG_END);
      }
    // Note that in some architectures a single memory operand can be
    // both read and written (for instance incl (%eax) on IA-32)
    // In that case we instrument it once for read and once for write.
    if (INS_MemoryOperandIsWritten(ins, memOp)) {
      INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)log_write_op,
                               IARG_INST_PTR, IARG_MEMORYOP_EA, memOp,
                               IARG_END);
      }
    }

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)log_instruction, IARG_PTR, id,
                 IARG_END);

}

VOID trace_after(ADDRINT ret)
{
	is_counting = false;
//	trace << instr_count << " " << mem_count << " " << std::endl;
	instr_count = 0;
	mem_count = 0;
}

VOID Image(IMG img, VOID* v)
{
	RTN processRtn = RTN_FindByName(img, "nf_core_process");
	if (RTN_Valid(processRtn)) {
		RTN_Open(processRtn);
		RTN_InsertCall(processRtn, IPOINT_BEFORE, (AFUNPTR) trace_before, IARG_ADDRINT, "nf_core_process", IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
		RTN_InsertCall(processRtn, IPOINT_AFTER, (AFUNPTR) trace_after, IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);
		RTN_Close(processRtn);
	} 
	else {
		//std::cout << "ERROR: could not find nf_core_process.." << std::endl;
	}
	

}

VOID Fini(INT32 code, VOID* v)
{
	trace.close();
}


int main(int argc, char* argv[])
{
	PIN_InitSymbols();
	if (PIN_Init(argc, argv)) {
		std::cout << "ERROR: could not init pin..." << std::endl;
		return 1;
	}

	trace.open("pincounts.log", ios::trunc);

	IMG_AddInstrumentFunction(Image, 0);
	INS_AddInstrumentFunction(trace_instr, 0);

	PIN_AddFiniFunction(Fini, 0);

	PIN_StartProgram();

	return 0;
}
