/* fixedx 单元测试：以 libm 的 double 结果为参照，按 LSB 断言误差上界。 */

#include <math.h>
#include <stdio.h>

#include "fixedx/fixedx.h"

static int failed = 0;

#define CHECK(cond)                                                    \
    do {                                                               \
        if (!(cond)) {                                                 \
            printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);     \
            failed++;                                                  \
        }                                                              \
    } while (0)

static int lsb_diff(double ref, fx_t got)
{
    double d = ref * FX_ONE;
    double err = got - d;
    return (int)(err >= 0 ? err : -err);
}

static void test_conversions(void)
{
    CHECK(fx_from_int(3) == 196608);
    CHECK(fx_from_int(-2) == -131072);
    CHECK(fx_to_double(fx_from_int(7)) == 7.0);
    CHECK(fx_from_double(1.5) == 98304);
    CHECK(fx_from_double(-1.5) == -98304);
}

static void test_arithmetic_exact(void)
{
    fx_t a = fx_from_double(1.5), b = fx_from_double(2.5);
    CHECK(fx_add(a, b) == fx_from_int(4));
    CHECK(fx_sub(a, b) == fx_from_int(-1));
    CHECK(fx_neg(a) == fx_from_double(-1.5));
}

static void test_mul_accuracy(void)
{
    /* 随机量与 double 参照比：±1 LSB */
    uint32_t seed = 0xC0FFEE;
    for (int i = 0; i < 2000; i++) {
        seed = seed * 1664525u + 1013904223u;
        double x = (double)(int32_t)seed / FX_ONE; /* 复用低位，简单伪随机 */
        seed = seed * 1664525u + 1013904223u;
        double y = (double)(int32_t)seed / FX_ONE;
        if (x > 200.0 || x < -200.0 || y > 200.0 || y < -200.0) {
            continue; /* 避免乘积超出 fx_t 范围 */
        }
        fx_t got = fx_mul(fx_from_double(x), fx_from_double(y));
        CHECK(lsb_diff(x * y, got) <= 1);
    }
    /* 手工抽查：1.5 * 2.5 = 3.75 */
    CHECK(fx_mul(fx_from_double(1.5), fx_from_double(2.5)) ==
          fx_from_double(3.75));
}

static void test_div_accuracy(void)
{
    CHECK(fx_div(fx_from_int(10), fx_from_int(4)) == fx_from_double(2.5));
    CHECK(fx_div(fx_from_int(1), fx_from_int(0)) == 0); /* 除零安全 */

    uint32_t seed = 0xABCDEF;
    for (int i = 0; i < 2000; i++) {
        seed = seed * 1664525u + 1013904223u;
        double x = ((double)(int32_t)seed / FX_ONE);
        seed = seed * 1664525u + 1013904223u;
        double y = ((double)(int32_t)seed / FX_ONE);
        if (y < 0.001 && y > -0.001) {
            continue; /* 躲开除零和巨商 */
        }
        if (x > 1000.0 || x < -1000.0) {
            continue;
        }
        fx_t got = fx_div(fx_from_double(x), fx_from_double(y));
        double ref = fx_from_double(x) / (double)fx_from_double(y);
        if (ref > 32000.0 || ref < -32000.0) {
            continue; /* 结果超出 fx_t 表示范围 */
        }
        CHECK(lsb_diff(ref, got) <= 1);
    }
}

static void test_sqrt(void)
{
    CHECK(fx_sqrt(0) == 0);
    CHECK(fx_sqrt(fx_from_int(-4)) == 0); /* 负数安全 */

    double cases[] = {1.0,    2.0,   4.0, 0.25,
                      0.0002, 1234.5, 3.14159, 15241.38393};
    for (int i = 0; i < 8; i++) {
        /* 参照对「量化后的输入」计算：0.0002 这类数转 fx 本身就有
         * 输入量化误差，那是转换的事，不是开方算法的事 */
        fx_t in = fx_from_double(cases[i]);
        fx_t got = fx_sqrt(in);
        CHECK(lsb_diff(sqrt(fx_to_double(in)), got) <= 2);
    }
    /* 往返一致性按相对误差断言：(sqrt(x))^2 / x 偏差 < 1e-4
     * （sqrt 的 2 LSB 误差经平方放大后是 4550 fx，不能用绝对 LSB 断言） */
    fx_t x = fx_from_double(1234.5);
    fx_t rt = fx_sqrt(x);
    double ratio = fx_to_double(fx_mul(rt, rt)) / 1234.5;
    CHECK(ratio > 0.9999 && ratio < 1.0001);
}

static void test_sin_cos_accuracy(void)
{
    double max_err_sin = 0, max_err_cos = 0;
    for (int i = 0; i <= 1000; i++) {
        double x = -2 * 3.14159265358979 + i * (4 * 3.14159265358979) / 1000;
        fx_t s = fx_sin(fx_from_double(x));
        fx_t c = fx_cos(fx_from_double(x));
        double es = s - sin(x) * FX_ONE;
        double ec = c - cos(x) * FX_ONE;
        if (es < 0) es = -es;
        if (ec < 0) ec = -ec;
        if (es > max_err_sin) max_err_sin = es;
        if (ec > max_err_cos) max_err_cos = ec;
        CHECK(es <= 6);
        CHECK(ec <= 6);
    }
    printf("  sin 最大误差 %.2f LSB, cos 最大误差 %.2f LSB\n", max_err_sin,
           max_err_cos);
}

static void test_sin_symmetry(void)
{
    /* sin(pi - x) = sin(x)，sin(-x) = -sin(x) */
    fx_t x = fx_from_double(1.0);
    CHECK(fx_sin(fx_sub(FX_PI, x)) == fx_sin(x));
    CHECK(fx_sin(fx_neg(x)) == -fx_sin(x));
    CHECK(fx_sin(0) == 0);
    CHECK(fx_cos(0) == FX_ONE); /* cos(0) = 1.0 */
}

int main(void)
{
    test_conversions();
    test_arithmetic_exact();
    test_mul_accuracy();
    test_div_accuracy();
    test_sqrt();
    test_sin_cos_accuracy();
    test_sin_symmetry();

    if (failed) {
        printf("FAILED: %d check(s)\n", failed);
        return 1;
    }
    printf("ALL PASSED\n");
    return 0;
}
