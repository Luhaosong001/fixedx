# fixedx

Q16.16 定点数学库（C99）：给没有 FPU 的单片机用的实数运算。

`fx_t` 就是一个 `int32_t`：高 16 位整数、低 16 位小数，分辨率
1/65536 ≈ 0.0000153，范围约 ±32767.99。没有浮点协处理器的芯片上，
一套 `float` 乘法要几十到上百个周期，定点乘法只要一条指令。

## 精度承诺（全部有测试背书）

| 运算 | 误差上界 | 测法 |
| --- | --- | --- |
| `fx_mul` / `fx_div` | ±1 LSB | 2000 组伪随机数对照 `double` |
| `fx_sqrt` | ±2 LSB | 牛顿迭代，8 个典型值对照 `double` |
| `fx_sin` / `fx_cos` | ±6 LSB（实测 ≤3.1） | 1000 个采样点覆盖 [-2π, 2π] 对照 libm |

`sin/cos` 用 128 项查找表 + 线性插值，表由
[tools/gen_sintab.py](tools/gen_sintab.py) 生成（`src/sintab.inc`），
任何人重跑脚本都能复现同一张表。

## 快速开始

```bash
make test    # 或手动编译：
gcc -std=c99 -Wall -Wextra -Werror -O2 -Iinclude tests/test_fixedx.c src/fixedx.c -o test_fixedx -lm
./test_fixedx
```

## 使用示例

```c
#include "fixedx/fixedx.h"

fx_t gravity = fx_from_double(9.8);   /* 初始化期用 double 转换 */
fx_t t = fx_from_int(2);

fx_t v = fx_mul(gravity, t);          /* v = 19.6，纯整数运算 */
fx_t h = fx_div(v, fx_from_int(2));   /* h = 9.8 */

fx_t angle = fx_div(fx_from_int(30), fx_from_int(180)); /* 弧度比例 */
fx_t s = fx_sin(angle);               /* 无 FPU 算正弦 */
```

## 实现要点

- **乘除用 64 位中间量**：`(int64)a * b >> 16`，不丢精度不溢出
- **开方是牛顿迭代**：初值保证不小于真值从而单调收敛；除法自带截断噪声，
  判停条件写成「不再下降」而不是「严格相等」——这是调试踩出来的细节
- **三角函数先归约再查表**：`[-π,π]` → 象限折叠 → `[0,π/2]`，
  sin 的奇对称和 sin(π-x)=sin(x) 都有对称性测试
- **除零安全**：`fx_div` 除数为 0 返回 0，不崩溃不未定义

## 已知取舍

- Q16.16 表达不了大于 ±32767.99 的数；需要更宽范围可以按同样套路做 Q8.24 / Q4.28
- `fx_from_double` / `fx_to_double` 是给测试和离线初始化用的桥接函数，
  单片机正式代码里不要出现 `double`

## License

MIT
