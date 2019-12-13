//XXX: fpuのcopy (for performance) (as of 25 Nov)

#include <cmath>
#include <cstdint>

#include "fpu.hh"
#include "bit.hh"

//extern union bits b;

uint32_t sqrt_init_m(uint32_t emod2, uint32_t m) {
    b.ui32 = (m & MUSE(MINPREC)) | (emod2 ? 0x3f800000 : 0x40000000);
    b.f = sqrtf(b.f);
    uint32_t m_ = b.ui32 & MUSE(MOUTPREC);
    return m_;
}

uint32_t sqrt_init_u(float f) {
    b.f = f;
    uint32_t u = b.ui32;
    uint32_t s = u & 0x80000000, e = (u >> 23) & 0x000000ff, m = u & 0x007fffff;
    if (e == 0) return 0;
    uint32_t m_ = sqrt_init_m(e & 1, m);
    uint32_t e_ = ((e - 1) >> 1) + 64;
    uint32_t u_ = s | (e_ << 23) | m_;
    return u_;
}

float sqrt_init(float f) {
    b.ui32 = sqrt_init_u(f);
    return b.f;
}

uint32_t finv_init_m(uint32_t m) {
    b.ui32 = (m & MUSE(MINPREC)) | 0x3f800000;
    b.f = 2.0f / b.f;
    uint32_t m_ = b.ui32 & MUSE(MOUTPREC);
    return m_;
}

uint32_t finv_init_u(float f) {
    b.f = f;
    uint32_t u = b.ui32;
    uint32_t s = u & 0x80000000, e = (u >> 23) & 0x000000ff, m = u & 0x007fffff;
    if (e == 0) return 0;
    uint32_t m_ = finv_init_m(m);
    uint32_t e_ = ((253 - e) & 0x000000ff) + (m & MUSE(MINPREC) ? 0 : 1);
    uint32_t u_ = s | (e_ << 23) | m_;
    return u_;
}

float finv_init(float f) {
    b.ui32 = finv_init_u(f);
    return b.f;
}

float mfinv(float x, float init) {
    float t = init;
    for (int i = 0; i < FINV_LOOP_COUNT; i++) {
        t = t * (2 - x * t);
    }
    return t;
}
//XXX: end of copy
