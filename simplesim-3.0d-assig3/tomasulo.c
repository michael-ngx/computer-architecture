
#include <limits.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "host.h"
#include "misc.h"
#include "machine.h"
#include "regs.h"
#include "memory.h"
#include "loader.h"
#include "syscall.h"
#include "dlite.h"
#include "options.h"
#include "stats.h"
#include "sim.h"
#include "decode.def"

#include "instr.h"

/* PARAMETERS OF THE TOMASULO'S ALGORITHM */

#define INSTR_QUEUE_SIZE         16

#define RESERV_INT_SIZE    5
#define RESERV_FP_SIZE     3
#define FU_INT_SIZE        3
#define FU_FP_SIZE         1

#define FU_INT_LATENCY     5
#define FU_FP_LATENCY      7

/* IDENTIFYING INSTRUCTIONS */

//unconditional branch, jump or call
#define IS_UNCOND_CTRL(op) (MD_OP_FLAGS(op) & F_CALL || \
                         MD_OP_FLAGS(op) & F_UNCOND)

//conditional branch instruction
#define IS_COND_CTRL(op) (MD_OP_FLAGS(op) & F_COND)

//floating-point computation
#define IS_FCOMP(op) (MD_OP_FLAGS(op) & F_FCOMP)

//integer computation
#define IS_ICOMP(op) (MD_OP_FLAGS(op) & F_ICOMP)

//load instruction
#define IS_LOAD(op)  (MD_OP_FLAGS(op) & F_LOAD)

//store instruction
#define IS_STORE(op) (MD_OP_FLAGS(op) & F_STORE)

//trap instruction
#define IS_TRAP(op) (MD_OP_FLAGS(op) & F_TRAP) 

#define USES_INT_FU(op) (IS_ICOMP(op) || IS_LOAD(op) || IS_STORE(op))
#define USES_FP_FU(op) (IS_FCOMP(op))

#define WRITES_CDB(op) (IS_ICOMP(op) || IS_LOAD(op) || IS_FCOMP(op))

/* FOR DEBUGGING */

//prints info about an instruction
#define PRINT_INST(out,instr,str,cycle)	\
  myfprintf(out, "%d: %s", cycle, str);		\
  md_print_insn(instr->inst, instr->pc, out); \
  myfprintf(stdout, "(%d)\n",instr->index);

#define PRINT_REG(out,reg,str,instr) \
  myfprintf(out, "reg#%d %s ", reg, str);	\
  md_print_insn(instr->inst, instr->pc, out); \
  myfprintf(stdout, "(%d)\n",instr->index);

/* VARIABLES */

//instruction queue for tomasulo
static instruction_t* instr_queue[INSTR_QUEUE_SIZE];
//number of instructions in the instruction queue
static int instr_queue_size = 0;
static int instr_queue_head = 0;

//reservation stations (each reservation station entry contains a pointer to an instruction)
static instruction_t* reservINT[RESERV_INT_SIZE];
static instruction_t* reservFP[RESERV_FP_SIZE];

//functional units
static instruction_t* fuINT[FU_INT_SIZE];
static instruction_t* fuFP[FU_FP_SIZE];

//common data bus
static instruction_t* commonDataBus = NULL;

//The map table keeps track of which instruction produces the value for each register
static instruction_t* map_table[MD_TOTAL_REGS];

//the index of the last instruction fetched
static int fetch_index = 0;


/* 
 * Description: 
 * 	Checks if simulation is done by finishing the very last instruction
 *      Remember that simulation is done only if the entire pipeline is empty
 * Inputs:
 * 	sim_insn: the total number of instructions simulated
 * Returns:
 * 	True: if simulation is finished
 */
static bool is_simulation_done(counter_t sim_insn) {

  /* ECE552 Assignment 3 - BEGIN CODE */

  return true; //ECE552: you can change this as needed; we've added this so the code provided to you compiles
}

/* 
 * Description: 
 * 	Retires the instruction from writing to the Common Data Bus
 * Inputs:
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void CDB_To_retire(int current_cycle) {

  /* ECE552 Assignment 3 - BEGIN CODE */

}


/* 
 * Description: 
 * 	Moves an instruction from the execution stage to common data bus (if possible)
 * Inputs:
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void execute_To_CDB(int current_cycle) {

  /* ECE552 Assignment 3 - BEGIN CODE */

}

/* 
 * Description: 
 * 	Moves instruction(s) from the issue to the execute stage (if possible). We prioritize old instructions
 *      (in program order) over new ones, if they both contend for the same functional unit.
 *      All RAW dependences need to have been resolved with stalls before an instruction enters execute.
 * Inputs:
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void issue_To_execute(int current_cycle) {

  /* ECE552 Assignment 3 - BEGIN CODE */
}

/* 
 * Description: 
 * 	Moves instruction(s) from the dispatch stage to the issue stage
 * Inputs:
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void dispatch_To_issue(int current_cycle) {

  /* ECE552 Assignment 3 - BEGIN CODE */
  if (instr_queue_size == 0) {
    return;
  }
  // Always try to dispatch the oldest instruction first
  instruction_t* instr = instr_queue[instr_queue_size-1];

  // Default to no hazard
  bool hazard = false;

  if (USES_INT_FU(instr->op)) {
    int i;
    for (i = 0; i < RESERV_INT_SIZE; i++) {
      if (reservINT[i] == NULL) {
        reservINT[i] = instr;
        break;
      }
    }
    if (i == RESERV_INT_SIZE) {
      hazard = true;
    }
  }
  else if (USES_FP_FU(instr->op)) {
    int i;
    for (i = 0; i < RESERV_FP_SIZE; i++) {
      if (reservFP[i] == NULL) {
        reservFP[i] = instr;
        break;
      }
    }
    if (i == RESERV_FP_SIZE) {
      hazard = true;
    }
  }
  else if (IS_UNCOND_CTRL(instr->op)) {

  }
  else if (IS_COND_CTRL(instr->op)) {

  }
  else {
    // Should never get here
    assert(1);
  }

  // NOTE: A fetched instruction can be dispatched (complete D) immediately 
  // if a reservation station entry will be available in the next cycle.
  
  if (hazard) return;

  // If a hazard was not detected, remove the instruction from the queue
  // This means the instruction has ALREADY been moved to a reservation station
  instr->Q[0] = map_table[instr->r_in[0]];
  instr->Q[1] = map_table[instr->r_in[1]];
  instr->Q[2] = map_table[instr->r_in[2]];
  map_table[instr->r_out[0]] = instr;
  map_table[instr->r_out[1]] = instr;

  instr_queue[instr_queue_size-1] = NULL;
  instr_queue_size--;
}

/* 
 * Description: 
 * 	Grabs an instruction from the instruction trace (if possible)
 * Inputs:
 *      trace: instruction trace with all the instructions executed
 * Returns:
 * 	None
 */
void fetch(instruction_trace_t* trace, int current_cycle) {
  /* ECE552 Assignment 3 - BEGIN CODE */
  if (instr_queue_size < INSTR_QUEUE_SIZE) {
    // Fetch until no trap instructions is found
    while (true) {
      instruction_t* instr = get_instr(trace, fetch_index++);
      if (IS_TRAP(instr->op)) {
        continue;
      }
    }
    
    // Move all current instructions in the queue up one (guaranteed to be space)
    int i;
    for (i = instr_queue_size; i > 0; i--) {
      instr_queue[i] = instr_queue[i-1];
    }
    instr_queue[0] = instr;
    instr->tom_dispatch_cycle = current_cycle;
    instr_queue_size++;
  }
  /* ECE552 Assignment 3 - END CODE */
}

/* 
 * Description: 
 * 	Calls fetch and dispatches an instruction at the same cycle (if possible)
 * Inputs:
 *      trace: instruction trace with all the instructions executed
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void fetch_To_dispatch(instruction_trace_t* trace, int current_cycle) {

  /* ECE552 Assignment 3 - BEGIN CODE */
  fetch(trace, current_cycle);
  /* ECE552 Assignment 3 - END CODE */
}

/* 
 * Description: 
 * 	Performs a cycle-by-cycle simulation of the 4-stage pipeline
 * Inputs:
 *      trace: instruction trace with all the instructions executed
 * Returns:
 * 	The total number of cycles it takes to execute the instructions.
 * Extra Notes:
 * 	sim_num_insn: the number of instructions in the trace
 */
counter_t runTomasulo(instruction_trace_t* trace)
{
  //initialize instruction queue
  int i;
  for (i = 0; i < INSTR_QUEUE_SIZE; i++) {
    instr_queue[i] = NULL;
  }

  //initialize reservation stations
  for (i = 0; i < RESERV_INT_SIZE; i++) {
    reservINT[i] = NULL;
  }

  for(i = 0; i < RESERV_FP_SIZE; i++) {
    reservFP[i] = NULL;
  }

  //initialize functional units
  for (i = 0; i < FU_INT_SIZE; i++) {
    fuINT[i] = NULL;
  }

  for (i = 0; i < FU_FP_SIZE; i++) {
    fuFP[i] = NULL;
  }

  //initialize map_table to no producers
  int reg;
  for (reg = 0; reg < MD_TOTAL_REGS; reg++) {
    map_table[reg] = NULL;
  }
  
  int cycle = 1;
  while (true) {
    /* ECE552 Assignment 3 - BEGIN CODE */
    CDB_To_retire(cycle);
    execute_To_CDB(cycle);
    issue_To_execute(cycle);
    dispatch_To_issue(cycle);
    fetch_To_dispatch(trace, cycle);
    /* ECE552 Assignment 3 - END CODE */
    cycle++;

    if (is_simulation_done(sim_num_insn))
      break;
  }
  
  return cycle;
}
