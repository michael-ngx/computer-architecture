#include "predictor.h"

static const UINT32 MAX_VALUE = 3; // 0: NN, 1: NT, 2: TN, 3: TT

/////////////////////////////////////////////////////////////
// 2bitsat
/////////////////////////////////////////////////////////////
static UINT32 twobitsat_prediction_table[4096] = {0};

void InitPredictor_2bitsat() {}

bool GetPrediction_2bitsat(UINT32 PC) {
  UINT32 prediction = twobitsat_prediction_table[PC & 0xFFF];
  if (prediction <= 1) return NOT_TAKEN;
  else return TAKEN;
}

void UpdatePredictor_2bitsat(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {
  UINT32 predicted = twobitsat_prediction_table[PC & 0xFFF];
  UINT32 updated;
  if (resolveDir == TAKEN) {
    updated = SatIncrement(predicted, MAX_VALUE);
  }
  else {
    updated = SatDecrement(predicted);
  }
  twobitsat_prediction_table[PC & 0xFFF] = updated;
}

/////////////////////////////////////////////////////////////
// 2level
/////////////////////////////////////////////////////////////
static const INT32 BHT_BITS = 9;
static const INT32 BHT_WIDTH = 6;
static const INT32 BHT_ROW_COUNT = 512;
static UINT32 BHT[BHT_ROW_COUNT] = {0};

static const INT32 PHT_BITS = 3;
static const INT32 PHT_TABLE_COUNT = 8;
static const INT32 PHT_ROW_COUNT = 64;
static UINT32 PHT[PHT_TABLE_COUNT][PHT_ROW_COUNT] = {0};

void InitPredictor_2level() {
}

bool GetPrediction_2level(UINT32 PC) {
  UINT32 history = BHT[(PC & 0x1FF) >> PHT_BITS] & 0x3F;
  UINT32 table = PC & 0b111;
  UINT32 prediction = PHT[table][history];
  if (prediction <= 1) return NOT_TAKEN;
  else return TAKEN;
}

void UpdatePredictor_2level(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {
  UINT32 history = BHT[(PC & 0x1FF) >> PHT_BITS] & 0x3F;
  UINT32 table = PC & 0b111;
  if (resolveDir == TAKEN) {
    PHT[table][history] = SatIncrement(PHT[table][history], MAX_VALUE);
    BHT[(PC & 0x1FF) >> PHT_BITS] = (history << 1) & 1;
  }
  else {
    PHT[table][history] = SatDecrement(PHT[table][history]);
    BHT[(PC & 0x1FF) >> PHT_BITS] = (history << 1);
  }
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

