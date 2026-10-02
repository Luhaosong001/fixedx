#ifndef FIXEDX_FIXEDX_H
#define FIXEDX_FIXEDX_H

#include <stdint.h>

/* fixedx：Q16.16 定点数学库，给没有 FPU 的单片机用。
 *
 * fx_t 用 32 位整数表示实数：高 16 位整数、低 16 位小数，
 * 分辨率 1/65536 ≈ 0.0000153，范围约 ±32767.99。
 *
 * 所有运算都有对应的单元测试，误差以 LSB（= 1/65536）为单位断言：
 * 乘除 ±1 LSB，开方 ±2 LSB，sin/cos ±6 LSB（128 项查表 + 线性插值）。
 */

typedef int32_t fx_t;

#define FX_ONE 65536               /* 1.0 */
#define FX_HALF_PI 102944          /* pi/2 */
#define FX_PI 205887               /* pi */

/* 整数/双精度转定点：fx_from_double 主要给测试和离线初始化用 */
fx_t fx_from_int(int v);
fx_t fx_from_double(double v);
double fx_to_double(fx_t v);       /* 同上，host 侧参照用 */

fx_t fx_add(fx_t a, fx_t b);
fx_t fx_sub(fx_t a, fx_t b);
fx_t fx_neg(fx_t a);
fx_t fx_mul(fx_t a, fx_t b);       /* 64 位中间量，±1 LSB */
fx_t fx_div(fx_t a, fx_t b);       /* 除数为 0 返回 0 */

fx_t fx_sqrt(fx_t v);              /* 牛顿迭代；负数返回 0，±2 LSB */
fx_t fx_sin(fx_t rad);             /* 弧度，±6 LSB */
fx_t fx_cos(fx_t rad);             /* = fx_sin(rad + pi/2) */

#endif
