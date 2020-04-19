import sys
import os.path
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

file_path = sys.argv[1]
tests = int(sys.argv[2])
plot_out_path = sys.argv[3]
csv_path = sys.argv[4] if (len(sys.argv)>=5) else 'csv.csv'

f = open(file_path)
data = [float(d.strip('\n')) for d in f.readlines()]
f.close()
avgs = [sum(data[i-tests:i]) / tests for i in range(tests,len(data) + 1,tests)]

threads = [1, 2, 4, 8, 16, 32]
kern = [3, 5, 7, 9, 11, 13, 15]

sequentialAvailable = os.path.isfile('Blur/results/blur_effect_sequential/sequential.out')

plt.figure(figsize=(8,10))
if(sequentialAvailable):
    plt.subplot(211)

for i,k in enumerate(kern):
    times = avgs[(i*len(threads)) : ((i+1)*len(threads))]
    plt.plot(threads, times, label=str(k))

plt.xticks(threads)
plt.ylabel('time')
plt.legend(title='Kernel Size')

if(sequentialAvailable):
    f = open('Blur/results/blur_effect_sequential/sequential.out')
    data = [float(d.strip('\n')) for d in f.readlines()]
    f.close()
    seq = [sum(data[i-tests:i]) / tests for i in range(tests,len(data) + 1,tests)]
    plt.subplot(212)
    
    ke = []
    t = []
    sped = []
    for i,k in enumerate(kern):
        times = avgs[(i*len(threads)) : ((i+1)*len(threads))]
        speedup =[seq[i] / p for p in times]
        ke += [k]*len(times)
        sped += speedup
        t+=times

        plt.plot(threads, speedup, label=str(k))
    df = pd.DataFrame({'Kernel': ke, 'Time': t, 'Speedup': sped}, columns = ['Kernel', 'Time', 'Speedup'])
    df.to_csv(csv_path, index = False, header = True, encoding='utf-8')
    plt.ylabel(r'speedup $\frac{sequential(t)}{paralell(t)}$')
    plt.xticks(threads)

plt.xlabel('threads')
plt.tight_layout()
plt.savefig(plot_out_path, bbox_inches='tight')
