#include "fixedx/fixedx.h"

#include "sintab.inc" /* 由 tools/gen_sintab.py 生成，128 项覆盖 [0, pi/2] */

fx_t fx_from_int(int v)
{
    return (fx_t)((int64_t)v * FX_ONE);
}

fx_t fx_from_double(double v)
{
    return (fx_t)(v * FX_ONE + ((v >= 0.0) ? 0.5 : -0.5));
}

double fx_to_double(fx_t v)
{
    return (double)v / FX_ONE;
}

fx_t fx_add(fx_t a, fx_t b)
{
    return a + b;
}

fx_t fx_sub(fx_t a, fx_t b)
{
    return a - b;
}

fx_t fx_neg(fx_t a)
{
    return -a;
}

fx_t fx_mul(fx_t a, fx_t b)
{
    return (fx_t)(((int64_t)a * b) >> 16);
}

fx_t fx_div(fx_t a, fx_t b)
{
    if (b == 0) {
        return 0;
    }
    return (fx_t)(((int64_t)a << 16) / b);
}

fx_t fx_sqrt(fx_t v)
{
    if (v <= 0) {
        return 0;
    }
    /* 初值必须 >= 真值，牛顿迭代单调下降收敛；
     * nx 不再下降说明已到 ±1 LSB 内（fx_div 有截断噪声） */
    uint32_t x = (v >= FX_ONE) ? (uint32_t)v : (uint32_t)FX_ONE;
    for (int i = 0; i < 32; i++) {
        uint32_t nx = (uint32_t)((x + (uint32_t)fx_div(v, (fx_t)x)) >> 1);
        if (nx >= x) {
            break;
        }
        x = nx;
    }
    return (fx_t)x;
}

/* 查表 + 线性插值：rad 必须在 [0, pi/2] 内 */
static fx_t sin_lookup(fx_t rad)
{
    /* 把 rad 映射到表下标 [0, 127]，分数部分做插值 */
    int64_t num = (int64_t)rad * (int64_t)(127);
    int idx = (int)(num / FX_HALF_PI);
    if (idx >= 127) {
        return SIN_TAB[127];
    }
    int64_t rem = num - (int64_t)idx * FX_HALF_PI;
    return SIN_TAB[idx] +
           (fx_t)((int64_t)(SIN_TAB[idx + 1] - SIN_TAB[idx]) * rem / FX_HALF_PI);
}

fx_t fx_sin(fx_t rad)
{
    /* 归约到 [-pi, pi] */
    while (rad > FX_PI) {
        rad -= 2 * FX_PI;
    }
    while (rad < -FX_PI) {
        rad += 2 * FX_PI;
    }
    int neg = rad < 0; /* sin(-x) = -sin(x) */
    if (neg) {
        rad = -rad;
    }
    if (rad > FX_HALF_PI) { /* sin(pi - x) = sin(x) */
        rad = FX_PI - rad;
    }
    fx_t v = sin_lookup(rad);
    return neg ? -v : v;
}

fx_t fx_cos(fx_t rad)
{
    return fx_sin(rad + FX_HALF_PI);
}
