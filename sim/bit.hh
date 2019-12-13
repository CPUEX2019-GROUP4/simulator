#pragma once

#include <cstdint>

//union bits {
//  float f;
//  uint32_t ui32;
//  struct {
//    uint16_t lo;
//    uint16_t hi;
//  } lohi;
//} b;

union bits {
  float f;
  uint32_t ui32;
  struct {
    uint16_t lo;
    uint16_t hi;
  } lohi;
};

extern union bits b;
