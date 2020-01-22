//XXX: fpuのcopy (for performance) (as of 15 Dec)

#include "fpu.hh"
#include <cmath>

uint32_t finv_init_m(uint32_t m) {
    b.ui32 = (m & MUSE(FINV_MINPREC)) | 0x3f800000;
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
    uint32_t e_ = ((253 - e) & 0x000000ff) + (m & MUSE(FINV_MINPREC) ? 0 : 1);
    uint32_t u_ = s | (e_ << 23) | m_;
    return u_;
}

float finv_init(float f) {
    b.ui32 = finv_init_u(f);
    return b.f;
}

uint32_t sqrt_inv_init_m(uint32_t emod2, uint32_t m) {
    b.ui32 = (m & MUSE(SQRT_MINPREC)) | (emod2 ? 0x3f800000 : 0x40000000);
    b.f = 2 / sqrtf(b.f);
    uint32_t m_ = b.ui32 & MUSE(MOUTPREC);
    return m_;
}

uint32_t sqrt_inv_init_u(float f) {
    b.f = f;
    uint32_t u = b.ui32;
    uint32_t s = u & 0x80000000, e = (u >> 23) & 0x000000ff, m = u & 0x007fffff;
    if (e == 0) return 0;
    uint32_t m_ = sqrt_inv_init_m(e & 1, m);
    uint32_t e_ = 189 - ((e - 1) >> 1) + (!(m & MUSE(SQRT_MINPREC)) && (e & 1) ? 1 : 0);
    uint32_t u_ = s | (e_ << 23) | m_;
    return u_;
}

//float sqrt_inv_init(float f) {
float sqrt_init(float f) {
    b.ui32 = sqrt_inv_init_u(f);
    return b.f;
}

//XXX: end of copy
