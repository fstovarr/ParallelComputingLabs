import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy import interpolate

MPI_DIR = 'Results/MPI'
SEQ_DIR = 'Results/Sequential'

all_files = ['test4k.jpga.out', 'test4k.jpgb.out']
seq_files = ['test4k.jpga.out', 'test4k.jpgb.out']

li = []

for filename in all_files:
    df = pd.read_csv('{0}/{1}'.format(SEQ_DIR, filename), index_col=None, sep="\t", header=None)
    li.append(df)

frame = pd.concat(li, axis=1, ignore_index=True)
frame.columns = ['instances', 'kernel', 'time']

time = frame.astype(float).groupby(['kernel', 'threads']).agg({'time': 'mean'})['time'].unstack()
time.plot()

plt.pause(10)
