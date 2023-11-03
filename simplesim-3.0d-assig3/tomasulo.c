
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
  for (int i = 0; i < INSTR_QUEUE_SIZE; i++) {
    if (instr_queue[i] != NULL) {
      return false;
    }
  }
  for (int i = 0; i < RESERV_INT_SIZE; i++) {
    if (reservINT[i] != NULL) {
      return false;
    }
  }
  for (int i = 0; i < RESERV_FP_SIZE; i++) {
    if (reservFP[i] != NULL) {
      return false;
    }
  }
  for (int i = 0; i < FU_INT_SIZE; i++) {
    if (fuINT[i] != NULL) {
      return false;
    }
  }
  for (int i = 0; i < FU_FP_SIZE; i++) {
    if (fuFP[i] != NULL) {
      return false;
    }
  }
  if (commonDataBus != NULL) {
    return false;
  }
  for (int i = 0; i < MD_TOTAL_REGS; i++) {
    if (map_table[i] != NULL) {
      return false;
    }
  }

  return true;
  /* ECE552 Assignment 3 - END CODE */
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
  if (commonDataBus == NULL) {
    return;
  }
  // Clear Q tags in other instructions
  for (int i = 0; i < RESERV_INT_SIZE; i++) {
    instruction_t* instr = reservINT[i];
    if (instr == NULL) continue;
    if (instr->Q[0] == commonDataBus) {
      instr->Q[0] = NULL;
    }
    if (instr->Q[1] == commonDataBus) {
      instr->Q[1] = NULL;
    }
    if (instr->Q[2] == commonDataBus) {
      instr->Q[2] = NULL;
    }
  }
  for (int i = 0; i < RESERV_FP_SIZE; i++) {
    instruction_t* instr = reservFP[i];
    if (instr == NULL) continue;
    if (instr->Q[0] == commonDataBus) {
      instr->Q[0] = NULL;
    }
    if (instr->Q[1] == commonDataBus) {
      instr->Q[1] = NULL;
    }
    if (instr->Q[2] == commonDataBus) {
      instr->Q[2] = NULL;
    }
  }

  // Clear entry in map table
  for (int i = 0; i < MD_TOTAL_REGS; i++) {
    if (map_table[i] == commonDataBus) {
      map_table[i] = NULL;
    }
  }

  commonDataBus = NULL;
  /* ECE552 Assignment 3 - END CODE */
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

  // Check if any instructions are ready to be written to CDB
  // Record all instructions that are ready into ready_instrs array
  instruction_t* ready_instrs[RESERV_INT_SIZE + RESERV_FP_SIZE];
  int ready_count = 0;
  for (int i = 0; i < FU_INT_SIZE; i++) {
    instruction_t* instr = fuINT[i];
    if (instr == NULL) continue;

    if (instr->tom_execute_cycle + FU_INT_LATENCY <= current_cycle) {
      ready_instrs[ready_count++] = instr;
    }
  }
  for (int i = 0; i < FU_FP_SIZE; i++) {
    instruction_t* instr = fuFP[i];
    if (instr == NULL) continue;

    if (instr->tom_execute_cycle + FU_FP_LATENCY <= current_cycle) {
      ready_instrs[ready_count++] = instr;
    }
  }

  // Sort instructions in ready_instrs by oldest first (smaller order)
  for (int i = 0; i < ready_count; i++) {
    for (int j = i+1; j < ready_count; j++) {
      if (ready_instrs[i]->index > ready_instrs[j]->index) {
        instruction_t* temp = ready_instrs[i];
        ready_instrs[i] = ready_instrs[j];
        ready_instrs[j] = temp;
      }
    }
  }

  // Only have 1 CDB, so only write the first non-store instruction in ready_instrs
  // All finished stores should be removed from reservation stations and functional units
  // (they do not do anything to map table since no output register)

  instruction_t* clear_rs_fu_instrs[RESERV_INT_SIZE + RESERV_FP_SIZE];
  int clear_rs_fu_count = 0;
  bool non_store_found = false;

  for (int i = 0; i < ready_count; i++) {
    instruction_t* instr = ready_instrs[i];
    if (IS_STORE(instr->op)) {
      clear_rs_fu_instrs[clear_rs_fu_count++] = instr;
    } 
    else if (!non_store_found) {
      commonDataBus = instr;
      instr->tom_cdb_cycle = current_cycle;
      clear_rs_fu_instrs[clear_rs_fu_count++] = instr;
      non_store_found = true;
    }
  }

  // Clear reservation stations and functional units
  for (int i = 0; i < clear_rs_fu_count; i++) {
    instruction_t* instr = clear_rs_fu_instrs[i];
    if (USES_INT_FU(instr->op)) {
      for (int j = 0; j < RESERV_INT_SIZE; j++) {
        if (reservINT[j] == instr) {
          reservINT[j] = NULL;
        }
      }
      for (int j = 0; j < FU_INT_SIZE; j++) {
        if (fuINT[j] == instr) {
          fuINT[j] = NULL;
        }
      }
    }
    else if (USES_FP_FU(instr->op)) {
      for (int j = 0; j < RESERV_FP_SIZE; j++) {
        if (reservFP[j] == instr) {
          reservFP[j] = NULL;
        }
      }
      for (int j = 0; j < FU_FP_SIZE; j++) {
        if (fuFP[j] == instr) {
          fuFP[j] = NULL;
        }
      }
    } 
  }
  
  /* ECE552 Assignment 3 - END CODE */
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

  /* INTEGER */
  int int_fu_free = 0;
  for (int i = 0; i < FU_INT_SIZE; i++) {
    if (fuINT[i] == NULL) {
      int_fu_free++;
    }
  }

  if (int_fu_free > 0) {
        // Get all instructions that are READY AND IS NOT YET EXECUTED, put into ready_instrs array
        instruction_t* ready_instrs[RESERV_INT_SIZE];
        int ready_count = 0;
        for (int i = 0; i < RESERV_INT_SIZE; i++) {
          instruction_t* instr = reservINT[i];
          if (instr == NULL) continue;

          // Need to add the check if this instruction is already issued to execute before
          if (instr->tom_execute_cycle != 0) continue;

          // Check if all RAW dependences have been resolved
          if (instr->Q[0] == NULL && instr->Q[1] == NULL && instr->Q[2] == NULL) {
            ready_instrs[ready_count++] = instr;
          }
        }

        // Sort instructions in ready_instrs by oldest first (smaller index)
        for (int i = 0; i < ready_count; i++) {
          for (int j = i+1; j < ready_count; j++) {
            if (ready_instrs[i]->index > ready_instrs[j]->index) {
              instruction_t* temp = ready_instrs[i];
              ready_instrs[i] = ready_instrs[j];
              ready_instrs[j] = temp;
            }
          }
        }

        // Move instructions to FUs (start executing this instruction)
        for (int i = 0; i < ready_count; i++) {
          instruction_t* instr = ready_instrs[i];
          if (instr == NULL) continue;
          // Assign to first available FU
          for (int j = 0; j < FU_INT_SIZE; j++) {
            if (fuINT[j] == NULL) {
              fuINT[j] = instr;
              instr->tom_execute_cycle = current_cycle;
              break;
            }
          }
        }
  }


  /* FLOAT */
  int fp_fu_free = 0;
  for (int i = 0; i < FU_FP_SIZE; i++) {
    if (fuFP[i] == NULL) {
      fp_fu_free++;
    }
  }

  if (fp_fu_free > 0) {
        // Get all instructions that are ready, put into ready_instrs array
        instruction_t* ready_instrs[RESERV_FP_SIZE];
        int ready_count = 0;
        for (int i = 0; i < RESERV_FP_SIZE; i++) {
          instruction_t* instr = reservFP[i];
          if (instr == NULL) continue;
          
          // Need to add the check if this instruction is already issued to execute before
          if (instr->tom_execute_cycle != 0) continue;

          // Check if all RAW dependences have been resolved
          if (instr->Q[0] == NULL && instr->Q[1] == NULL && instr->Q[2] == NULL) {
            ready_instrs[ready_count++] = instr;
          }
        }

        // Sort instructions in ready_instrs by oldest first (smaller index)
        for (int i = 0; i < ready_count; i++) {
          for (int j = i+1; j < ready_count; j++) {
            if (ready_instrs[i]->index > ready_instrs[j]->index) {
              instruction_t* temp = ready_instrs[i];
              ready_instrs[i] = ready_instrs[j];
              ready_instrs[j] = temp;
            }
          }
        }

        // Move instructions to FUs
        for (int i = 0; i < ready_count; i++) {
          instruction_t* instr = ready_instrs[i];
          if (instr == NULL) continue;
          // Assign to first available FU
          for (int j = 0; j < FU_FP_SIZE; j++) {
            if (fuFP[j] == NULL) {
              fuFP[j] = instr;
              instr->tom_execute_cycle = current_cycle;
              break;
            }
          }
        }
  }
  /* ECE552 Assignment 3 - END CODE */
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
        instr->tom_issue_cycle = current_cycle;
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
        instr->tom_issue_cycle = current_cycle;
        break;
      }
    }
    if (i == RESERV_FP_SIZE) {
      hazard = true;
    }
  }
  else if (IS_UNCOND_CTRL(instr->op)) {
    // No hazard, but do nothing
  }
  else if (IS_COND_CTRL(instr->op)) {
    // No hazard, but do nothing
  }
  else {
    // Should never get here
    assert(1);
  }
  
  if (hazard) return;

  // If a hazard was not detected, remove the instruction from the IFQ
  // This means the instruction has ALREADY been moved to a reservation station
  if (IS_UNCOND_CTRL(instr->op) || IS_COND_CTRL(instr->op)) {
    // Do not care about register values
  } 
  else {
    if (instr->r_in[0] > 0) instr->Q[0] = map_table[instr->r_in[0]];
    if (instr->r_in[1] > 0) instr->Q[1] = map_table[instr->r_in[1]];
    if (instr->r_in[2] > 0) instr->Q[2] = map_table[instr->r_in[2]];
    if (instr->r_out[0] > 0) map_table[instr->r_out[0]] = instr;
    if (instr->r_out[1] > 0) map_table[instr->r_out[1]] = instr;
  }
  instr_queue[instr_queue_size-1] = NULL;
  instr_queue_size--;
  /* ECE552 Assignment 3 - END CODE */
}

/* 
 * Description: 
 * 	Grabs an instruction from the instruction trace (if possible)
 * Inputs:
 *      trace: instruction trace with all the instructions executed
 * Returns:
 * 	None
 */
void fetch(instruction_trace_t* trace) {

  /* ECE552 Assignment 3 - BEGIN CODE */

  // Fetch until no trap instructions is found
  instruction_t* instr;
  do {
    instr = get_instr(trace, ++fetch_index);
  } while (IS_TRAP(instr->op));

  // Move all current instructions in the queue up one
  int i;
  for (i = instr_queue_size; i > 0; i--) {
    instr_queue[i] = instr_queue[i-1];
  }
  instr_queue[0] = instr;
  instr->index = fetch_index;
  instr_queue_size++;
  
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

  // Do not fetch new instruction if program is over
  if (fetch_index >= sim_num_insn) return;

  if (instr_queue_size < INSTR_QUEUE_SIZE) {
    int save = instr_queue_size;
    fetch(trace);
    if (instr_queue_size > save) instr_queue[0]->tom_dispatch_cycle = current_cycle;
  }
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
