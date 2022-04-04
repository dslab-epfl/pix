#include "pin.H"
#include <iostream>
#include <fstream>
#include <string>

// This is more or less copied from the pintool examples

ofstream trace;
static bool is_counting = false;
static UINT64 instr_count = 0;
static UINT64 mem_count = 0;
CONTEXT* ctx; 

typedef struct {
  unsigned long ip;
  std::string function;
  std::string assembly;
  std::string category;
} instruction_data_t;

std::vector<std::pair<bool, unsigned long>> addresses;

VOID log_read_op(VOID *ip, VOID *addr) {
 if(is_counting)  addresses.push_back(std::make_pair(0, (unsigned long)addr));
}

VOID log_write_op(VOID *ip, VOID *addr) {
  if(is_counting) addresses.push_back(std::make_pair(1, (unsigned long)addr));
}

VOID log_instruction(CONTEXT* ctx, instruction_data_t *id) {
  if (is_counting) { 

  /* Printing instruction category */
  trace << id->category << std::endl;

  /* Printing values of all registers before the instruction was executed */
  trace << "rax = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_RAX) << std::endl ;
  trace << "rbx = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_RBX) << std::endl ;
  trace << "rdi = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_RDI) << std::endl ;
  trace << "rsi = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_RSI) << std::endl ;
  trace << "rdx = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_RDX) << std::endl ;
  trace << "rcx = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_RCX) << std::endl ;
  trace << "rbp = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_RBP) << std::endl ;
  trace << "rsp = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_RSP) << std::endl ;
  trace << "r8 = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_R8) << std::endl ;
  trace << "r9 = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_R9) << std::endl ;
  trace << "r10 = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_R10) << std::endl ;
  trace << "r11 = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_R11) << std::endl ;
  trace << "r12 = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_R12) << std::endl ;
  trace << "r13 = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_R13) << std::endl ;
  trace << "r14 = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_R14) << std::endl ;
  trace << "r15 = " << PIN_GetContextReg(ctx, LEVEL_BASE::REG_R15) << std::endl ;
//  trace << "flags = " <<  PIN_GetContextReg(ctx, LEVEL_BASE::REG_FLAGS) << std::endl;

  /* Printing instruction IP, Function name and assembly*/
  trace << std::hex << std::uppercase << id->ip << " |";

  trace  << id->function << " | " << id->assembly << " |";
 
  /* Printing memory addresses accessed by instruction*/
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
    id->category = CATEGORY_StringShort(INS_Category(ins));

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

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)log_instruction, IARG_CONTEXT, IARG_PTR, id,
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
	RTN processRtn = RTN_FindByName(img, "map_get");
	if (RTN_Valid(processRtn)) {
		RTN_Open(processRtn);
		RTN_InsertCall(processRtn, IPOINT_BEFORE, (AFUNPTR) trace_before, IARG_ADDRINT, "map_get", IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
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
