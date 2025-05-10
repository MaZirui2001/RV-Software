# 生成320个随机在16位范围内的整数，并每8个为一行打印，注意逗号分割
import random

# 生成320个随机在16位范围内的整数
data = [random.randint(0, 4) for _ in range(320)]

# 每8个为一行打印，注意逗号分割, 每行最后加一个逗号
for i in range(0, 29, 8):
    print(", ".join([str(data[j]) for j in range(i, i+8)]), end=",\n")
