import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy import interpolate

CUDA_DIR = 'Results/CUDA'
SEQ_DIR = 'Results/Sequential'

all_files = ['test4k.jpga.out', 'test4k.jpgb.out']
seq_files = ['test4k.jpga.out', 'test4k.jpgb.out']

li = []

for filename in all_files:
    df = pd.read_csv('{0}/{1}'.format(CUDA_DIR, filename), index_col=None, sep="\t", header=None)
    li.append(df)

frame = pd.concat(li, axis=1, ignore_index=True)
frame.columns = ['blocks', 'threads_per_block', 'kernel', 'time']

frame['threads'] = np.multiply(frame['blocks'], frame['threads_per_block'])

time = frame.astype(float).groupby(['kernel', 'threads']).agg({'time': 'mean'})['time'].unstack()
time.plot()

plt.pause(10)

li = []

for filename in all_files:
    seq = pd.read_csv('{0}/{1}'.format(SEQ_DIR, filename), index_col=None, sep="\t", header=None)
    li.append(seq)

seq_frame = pd.concat(li, axis=1, ignore_index=True)
seq_frame.columns = ['kernel', 'time']

seq_time = seq_frame.astype(float).groupby('kernel').agg({'time': 'mean'})['time']

x = seq_time.index
y = seq_time.values
f = interpolate.interp1d(x, y, fill_value="extrapolate")
x_gen = [3, 17, 31, 45]
time_gen = pd.Series(f(x_gen), index=x_gen)
time_gen.plot()

plt.pause(10)

speedup = (time_gen / time.T).T
speedup.plot()

plt.pause(20)

# import numpy as np
# from scipy import interpolate
# x = [1, 2, 3, 4, 5]
# y = [5, 10, 15, 20, 25]
# f = interpolate.interp1d(x, y, fill_value = "extrapolate")
# print(f(6))