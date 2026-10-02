#!/usr/bin/env python3
"""生成 fixedx 的 sin 查找表（Q16.16，128 项覆盖 [0, pi/2]）。

用法：python tools/gen_sintab.py > src/sintab.inc
表是线性插值用的，重跑本脚本必须与仓库里的 sintab.inc 完全一致。
"""
import math

N = 128
SCALE = 65536  # Q16.16

vals = [round(math.sin(i * (math.pi / 2) / (N - 1)) * SCALE) for i in range(N)]
assert vals[-1] == SCALE, "sin(pi/2) 必须恰好是 1.0"

print("static const fx_t SIN_TAB[%d] = {" % N)
for r in range(0, N, 8):
    print("    " + ", ".join(str(v) for v in vals[r:r + 8]) + ",")
print("};")
