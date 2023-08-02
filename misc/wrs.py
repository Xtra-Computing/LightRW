import math
import random
import matplotlib.pyplot as plt

wa =     [1,3,4,9,1,4,8,7,5]
#wa =     [1,1,1,1,1,1,1,1,1]
output = [0,0,0,0,0,0,0,0,0]

test_n = 100000
for j in range(test_n):
    w = 0
    res = 0
    for i in range(len(wa)):
        r = random.uniform(0,1)
        w = w + wa[i]
        if r <  wa[i]/w :
            res = i
    output[res] = output[res] + 1

output = [i / test_n for i in output]

sum_w = sum(wa)
target_w = [i /sum(wa) for i in wa]

plt.plot(output)

plt.plot(target_w)

plt.ylim([0, 1])

plt.show()