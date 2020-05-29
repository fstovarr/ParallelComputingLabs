import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy import interpolate

all_files = ['test4k.jpga.out', 'test4k.jpgb.out']

li = []

for filename in all_files:
    df = pd.read_csv(filename, index_col=None, sep="\t", header=None)
    li.append(df)

frame = pd.concat(li, axis=1, ignore_index=True)
frame.columns = ['kernel', 'time']

time = frame.astype(float).groupby('kernel').agg({'time': 'mean'})['time']
# time.plot()

x = time.index
y = time.values
f = interpolate.interp1d(x, y, fill_value="extrapolate")
x_gen = [3, 17, 31, 45]
time_gen = pd.Series(f(x_gen), index=x_gen)

time_gen.plot()

plt.pause(70)
