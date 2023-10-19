#include "predictor.h"

/////////////////////////////////////////////////////////////
// 2bitsat
/////////////////////////////////////////////////////////////
static const UINT32 MAX_VALUE = 3; // 0: NN, 1: NT, 2: TN, 3: TT

// (8192 bits) / (UINT32 per entry) = 256 entries (8 bits)
// (UINT32 per entry / 2 bits per prediction) = 16 2bit predictions per entry (4 bits)
static const PC_BITS = 12;
static UINT32 2bitsat_prediction_table[256] = {0};

void InitPredictor_2bitsat() {}

bool GetPrediction_2bitsat(UINT32 PC) {
  UINT32 prediction = 2bitsat_prediction_table[(PC & 0xFF0) >> 4] & (0b11 << (2 * (PC & 0xF)));
  if (prediction <= 1) return NOT_TAKEN;
  else return TAKEN;
}

void UpdatePredictor_2bitsat(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {
  UINT32 predicted = 2bitsat_prediction_table[(PC & 0xFF0) >> 4] & (0b11 << (2 * (PC & 0xF)));
  UINT32 updated;
  if (resolveDir == TAKEN) {
    updated = SatIncrement(predicted, MAX_VALUE);
  }
  else {
    updated = SatDecrement(predicted);
  }
  2bitsat_prediction_table[(PC & 0xFF0) >> 4] |= (updated << (2 * (PC & 0xF)));
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

