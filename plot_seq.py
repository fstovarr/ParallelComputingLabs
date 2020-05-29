import sys
import numpy as np
import matplotlib.pyplot as plt

file_path = sys.argv[1]
tests = int(sys.argv[2])
plot_out_path = sys.argv[3]

f = open(file_path)
data = [float(d.strip('\n')) for d in f.readlines()]
f.close()
avgs = [sum(data[i - tests :i]) / tests for i in range(tests, len(data) + 1, tests)]

print(avgs)

kern = [3, 5, 7, 9, 11, 13, 15]

plt.plot(kern, avgs)

plt.xticks(kern)
plt.xlabel('kern_size')
plt.ylabel('time')
# plt.show()
plt.savefig(plot_out_path, bbox_inches='tight')
