#ifndef MD5_H
#define MD5_H

#include <iostream>
#include <string>
#include <cstring>
#include <arm_neon.h>

using namespace std;

typedef unsigned char Byte;
typedef unsigned int bit32;

// 常数定义
#define s11 7
#define s12 12
#define s13 17
#define s14 22
#define s21 5
#define s22 9
#define s23 14
#define s24 20
#define s31 4
#define s32 11
#define s33 16
#define s34 23
#define s41 6
#define s42 10
#define s43 15
#define s44 21

// 原始串行宏 
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

#define ROTATELEFT(num, n) (((num) << (n)) | ((num) >> (32-(n))))

#define FF(a, b, c, d, x, s, ac) { \
  (a) += F ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
  (a) += G ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
  (a) += H ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
  (a) += I ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}

//  SIMD 向量化宏
#define vF(x, y, z) vorrq_u32(vandq_u32(x, y), vbicq_u32(z, x))
#define vG(x, y, z) vorrq_u32(vandq_u32(x, z), vbicq_u32(y, z))
#define vH(x, y, z) veorq_u32(veorq_u32(x, y), z)
#define vI(x, y, z) veorq_u32(y, vorrq_u32(x, vmvnq_u32(z)))

#define vROTATELEFT(num, n) vorrq_u32(vshlq_n_u32(num, n), vshrq_n_u32(num, 32 - n))

#define vFF(a, b, c, d, x, s, ac) { \
  a = vaddq_u32(a, vaddq_u32(vaddq_u32(vF(b, c, d), x), vdupq_n_u32(ac))); \
  a = vROTATELEFT(a, s); \
  a = vaddq_u32(a, b); \
}
#define vGG(a, b, c, d, x, s, ac) { \
  a = vaddq_u32(a, vaddq_u32(vaddq_u32(vG(b, c, d), x), vdupq_n_u32(ac))); \
  a = vROTATELEFT(a, s); \
  a = vaddq_u32(a, b); \
}
#define vHH(a, b, c, d, x, s, ac) { \
  a = vaddq_u32(a, vaddq_u32(vaddq_u32(vH(b, c, d), x), vdupq_n_u32(ac))); \
  a = vROTATELEFT(a, s); \
  a = vaddq_u32(a, b); \
}
#define vII(a, b, c, d, x, s, ac) { \
  a = vaddq_u32(a, vaddq_u32(vaddq_u32(vI(b, c, d), x), vdupq_n_u32(ac))); \
  a = vROTATELEFT(a, s); \
  a = vaddq_u32(a, b); \
}

// 函数声明
void MD5Hash(string input, bit32 *state);
// 找到这行并改成：
void MD5Hash_SIMD(const string* inputs[4], bit32 states[4][4]);
#endif
