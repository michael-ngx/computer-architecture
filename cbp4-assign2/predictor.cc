#include "predictor.h"

/////////////////////////////////////////////////////////////
// 2bitsat
/////////////////////////////////////////////////////////////
static UINT32 twoBitSat = 0;
static const UINT32 MAX_VALUE = 3; // 0: NN, 1: NT, 2: TN, 3: TT

void InitPredictor_2bitsat() {
}

bool GetPrediction_2bitsat(UINT32 PC) {
  if (twoBitSat <= 1) return NOT_TAKEN;
  else return TAKEN;
}

void UpdatePredictor_2bitsat(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {
  if (resolveDir == TAKEN) SatIncrement(twoBitSat, MAX_VALUE);
  else SatDecrement(twoBitSat);
}

/////////////////////////////////////////////////////////////
// 2level
/////////////////////////////////////////////////////////////
static const INT32 BHT_BITS = 9;
static const INT32 BHT_WIDTH = 6;
static const INT32 BHT_ROW_COUNT = 512;

static const INT32 PHT_BITS = 3;
static const INT32 PHT_TABLE_COUNT = 8;
static const INT32 PHT_ROW_COUNT = 64;

void InitPredictor_2level() {

}

bool GetPrediction_2level(UINT32 PC) {

  return TAKEN;
}

void UpdatePredictor_2level(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {

}

/////////////////////////////////////////////////////////////
// openend
/////////////////////////////////////////////////////////////

void InitPredictor_openend() {

}

bool GetPrediction_openend(UINT32 PC) {

  return TAKEN;
}

void UpdatePredictor_openend(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {

}

