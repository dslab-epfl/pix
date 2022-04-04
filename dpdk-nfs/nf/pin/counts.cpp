#include "pin.H"
#include <fstream>
#include <iostream>
#include <string>

std::ofstream trace;
static bool is_counting = false;
static UINT64 instr_count = 0;
static UINT64 mem_count = 0;

typedef struct {
  unsigned long ip;
  std::string function;
  std::string assembly;
} instruction_data_t;

std::string start_fn = "";
std::string end_fn = "";

std::vector<std::pair<bool, unsigned long>> addresses;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<std::string> KnobStartFn(KNOB_MODE_WRITEONCE, "pintool", "start-fn",
                         "nf_core_process",
                         "specify function at which to start tracing");
KNOB<std::string> KnobEndFn(KNOB_MODE_WRITEONCE, "pintool", "end-fn", "exit@plt",
                       "specify function at which to end tracing");

VOID log_read_op(VOID *ip, VOID *addr) {
  if (is_counting)
    addresses.push_back(std::make_pair(0, (unsigned long)addr));
}

VOID log_write_op(VOID *ip, VOID *addr) {
  if (is_counting)
    addresses.push_back(std::make_pair(1, (unsigned long)addr));
}

VOID log_instruction(instruction_data_t *id) {
  if (is_counting) {

    trace << std::hex << std::uppercase << id->ip << " |";

    trace << id->function << " | " << id->assembly << " |";

    for (auto a : addresses) {
      trace << " " << (a.first ? "w" : "r") << a.second;
    }
    addresses.clear();

    trace << std::endl;
  }
}

VOID doinstrcount() {
  if (is_counting) {
    instr_count++;
  }
}

VOID domemcount(VOID *ip, VOID *addr) {
  if (is_counting) {
    mem_count++;
  }
}

VOID trace_before(CHAR *name, ADDRINT size) { is_counting = true; }

VOID trace_instr(INS ins, VOID *v) {
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)doinstrcount, IARG_END);

  // Instruments memory accesses using a predicated call, i.e.
  // the instrumentation is called iff the instruction will actually be
  // executed.
  //
  // On the IA-32 and Intel(R) 64 architectures conditional moves and REP
  // prefixed instructions appear as predicated instructions in Pin.
  UINT32 memOperands = INS_MemoryOperandCount(ins);

  // Iterate over each memory operand of the instruction.
  for (UINT32 memOp = 0; memOp < memOperands; memOp++) {
    if (INS_MemoryOperandIsRead(ins, memOp) ||
        INS_MemoryOperandIsWritten(ins, memOp)) {
      INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)domemcount,
                               IARG_INST_PTR, IARG_MEMORYOP_EA, memOp,
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

VOID trace_after(ADDRINT ret) {
  is_counting = false;
  //	trace << instr_count << " " << mem_count << " " << std::endl;
  trace << "New Packet" << std::endl;
  instr_count = 0;
  mem_count = 0;
}

VOID Image(IMG img, VOID *v) {

  RTN processRtn = RTN_FindByName(img, start_fn.c_str());
  if (RTN_Valid(processRtn)) {
    RTN_Open(processRtn);
    RTN_InsertCall(processRtn, IPOINT_BEFORE, (AFUNPTR)trace_before,
                   IARG_ADDRINT, start_fn.c_str(),
                   IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
    RTN_Close(processRtn);
  } else {
    // std::cout << "ERROR: could not find dchain_allocate_new_index.." <<
    // std::endl;
  }
  processRtn = RTN_FindByName(img, end_fn.c_str());
  if (RTN_Valid(processRtn)) {
    RTN_Open(processRtn);
    RTN_InsertCall(processRtn, IPOINT_AFTER, (AFUNPTR)trace_after,
                   IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);
    RTN_Close(processRtn);
  }

  /* Some additional defaults */
  processRtn = RTN_FindByName(img, "ixgbe_xmit_pkts");
  if (RTN_Valid(processRtn)) {
    RTN_Open(processRtn);
    RTN_InsertCall(processRtn, IPOINT_AFTER, (AFUNPTR)trace_after,
                   IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);
    RTN_Close(processRtn);
  }
  processRtn = RTN_FindByName(img, "singleton_enqueue");
  if (RTN_Valid(processRtn)) {
    RTN_Open(processRtn);
    RTN_InsertCall(processRtn, IPOINT_AFTER, (AFUNPTR)trace_after,
                   IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);
    RTN_Close(processRtn);
  }
}

VOID Fini(INT32 code, VOID *v) { trace.close(); }

int main(int argc, char *argv[]) {
  trace.open("trace.out", std::ofstream::out);
  trace << "New Packet" << std::endl;
  PIN_InitSymbols();
  if (PIN_Init(argc, argv)) {
    std::cout << "ERROR: could not init pin..." << std::endl;
    return 1;
  }

  start_fn = KnobStartFn.Value();
  end_fn = KnobEndFn.Value();

  IMG_AddInstrumentFunction(Image, 0);
  INS_AddInstrumentFunction(trace_instr, 0);

  PIN_AddFiniFunction(Fini, 0);

  PIN_StartProgram();

  return 0;
}
