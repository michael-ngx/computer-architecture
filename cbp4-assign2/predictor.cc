#include "predictor.h"
#include <vector>
#include <tuple>
#include <cstdlib>
#include <ctime>

static const UINT32 MAX_VALUE = 3; // 0: NN, 1: NT, 2: TN, 3: TT

/////////////////////////////////////////////////////////////
// 2bitsat
/////////////////////////////////////////////////////////////
static UINT32 twobitsat_prediction_table[4096] = {0};

void InitPredictor_2bitsat() {
}

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
static UINT32 BHT[512] = {0};
static UINT32 PHT[8][64] = {0};

void InitPredictor_2level() {
}

bool GetPrediction_2level(UINT32 PC) {
  UINT32 history = BHT[(PC & 0xFF8) >> 3] & 0x3F;
  UINT32 table = PC & 0b111;
  UINT32 prediction = PHT[table][history];
  if (prediction <= 1) return NOT_TAKEN;
  else return TAKEN;
}

void UpdatePredictor_2level(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {
  UINT32 history = BHT[(PC & 0xFF8) >> 3] & 0x3F;
  UINT32 table = PC & 0b111;
  UINT32 predicted = PHT[table][history];

  PHT[table][history] = (resolveDir == TAKEN) ? SatIncrement(predicted, MAX_VALUE) : SatDecrement(predicted);
  BHT[(PC & 0xFF8) >> 3] = (history << 1) | ((resolveDir == TAKEN) ? TAKEN : NOT_TAKEN);
}

/////////////////////////////////////////////////////////////
// openend - ver 1: PPM-like, tag-based BP (avg 6.9)
/////////////////////////////////////////////////////////////
static const UINT32 MAX_COUNTER_CUSTOM = 7;  // 3-bit counter
static uint64_t global_history = 0;
static INT32 X = 0; // Last chosen bank

// static UINT32 base[4096] = {0};  // Each entry should be 4 bits
// static UINT32 banks[4][1024] = {0}; // Each entry should be 12 bits

static std::vector<std::tuple<UINT32,UINT32>> base(4096, {0,0});
static std::vector<std::vector<std::tuple<UINT32,UINT32,UINT32>>> banks(
  4,
  std::vector<std::tuple<UINT32,UINT32,UINT32>>(
    1024,
    {0,0,0}
  ))
;

// 8 bits
UINT32 get_tag(UINT32 PC) {
  return (PC & 0xFF) ^ (global_history & 0xFF);
}

// 10 bits
UINT32 get_index(UINT32 PC, INT32 bank) {
  if (bank == -1) return PC & 0xFFF;
  UINT32 index_0 = (PC & 0x3FF) ^ ((PC & 0xFFC00) >> 10) ^ (global_history & 0x3FF);
  UINT32 index_1 = index_0 ^ ((global_history & 0xFFC00) >> 10);
  UINT32 index_2 = index_1 ^ ((global_history & 0x3FF00000) >> 20);
  UINT32 index_3 = index_2 ^ ((global_history & 0xFFC0000000) >> 30);
  if (bank == 0) return index_0;
  else if (bank == 1) return index_1;
  else if (bank == 2) return index_2;
  else return index_3;
}

void InitPredictor_openend() {
  std::srand(static_cast<unsigned>(std::time(nullptr)));
}

bool GetPrediction_openend(UINT32 PC) {
  UINT32 tag = get_tag(PC);
  for (int i = 3; i >= 0; i--) {
    X = i;
    std::tuple<UINT32,UINT32,UINT32> entry = banks[i][get_index(PC, i)];
    if (get<1>(entry) == tag){
      return get<0>(entry) >= 4 ? TAKEN : NOT_TAKEN;
    }
  }
  X = -1;
  return (get<0>(base[get_index(PC, -1)]) >= 4) ? TAKEN : NOT_TAKEN;
}

void UpdatePredictor_openend(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {
  UINT32 index = get_index(PC, X);
  UINT32 tag = get_tag(PC);
  UINT32 base_index = get_index(PC, -1);
  UINT32 m = get<1>(base[base_index]);

  // Update 3 bit counter of X
  if (X == -1) {
    get<0>(base[index]) = resolveDir ? SatIncrement(get<0>(base[index]), MAX_COUNTER_CUSTOM) : SatDecrement(get<0>(base[index]));
  } else {
    get<0>(banks[X][index]) = resolveDir ? SatIncrement(get<0>(banks[X][index]), MAX_COUNTER_CUSTOM) : SatDecrement(get<0>(banks[X][index]));
  }

  // Allocate new entries
  if (resolveDir != predDir && X <= 2) {
    bool any = false;
    for (int Y = X + 1; Y <= 3; Y++) {
      UINT32 idx = get_index(PC, Y);
      std::tuple<UINT32,UINT32,UINT32> entry = banks[Y][idx];
      if (!get<2>(entry)) {
        any = true;
        get<1>(banks[Y][idx]) = tag;
        // Allocating counter
        get<0>(banks[Y][idx]) = m ? (resolveDir ? 4 : 3) : (get<0>(base[base_index]) ? 4 : 3);
      }
    }

    // Pick a random bank between [X + 1, 3]
    if (!any) {
      int Y = std::rand() % (3 - X) + (X + 1);
      UINT32 idx = get_index(PC, Y);
      any = true;
      get<1>(banks[Y][idx]) = tag;
      get<0>(banks[Y][idx]) = m ? (resolveDir ? 4 : 3) : (get<0>(base[base_index]) ? 4 : 3);
    }
  }

  // Update u and m
  if (X != -1) {
    get<2>(banks[X][index]) = (resolveDir == predDir) ? TAKEN : NOT_TAKEN;
    get<1>(base[base_index]) = (resolveDir == predDir) ? TAKEN : NOT_TAKEN;
  }
  
  // Update global history
  global_history = (global_history << 1) | (resolveDir == TAKEN ? TAKEN : NOT_TAKEN);
}

/*
bool GetPrediction_openend(UINT32 PC) {
  index = get_index(PC,4);
  if ((banks[3][index] & 0x1FE) >> 1 == get_tag(PC)){
    X = 4;
    return ((banks[3][index] & 0xE00) >> 9) >= 4 ? TAKEN : NOT_TAKEN;
  }
  index = get_index(PC,3);
  if ((banks[2][index] & 0x1FE) >> 1 == get_tag(PC)) {
    X = 3;
    return ((banks[2][index] & 0xE00) >> 9) >= 4 ? TAKEN : NOT_TAKEN;
  }
  index = get_index(PC,2);
  if ((banks[1][index] & 0x1FE) >> 1 == get_tag(PC)) {
    X = 2;
    return ((banks[1][index] & 0xE00) >> 9) >= 4 ? TAKEN : NOT_TAKEN;
  }
  index = get_index(PC,1);
  if ((banks[0][index] & 0x1FE) >> 1 == get_tag(PC)) {
    X = 1;
    return ((banks[0][index] & 0xE00) >> 9) >= 4 ? TAKEN : NOT_TAKEN;
  }
  X = 0;
  return (((base[PC & 0xFFF] & 0b1110) >> 1) >= 4) ? TAKEN : NOT_TAKEN;
}*/

// void UpdatePredictor_openend(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {
// }