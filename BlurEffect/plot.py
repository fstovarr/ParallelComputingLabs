import pandas as pd
data = pd.read_csv('out.out', sep=" ", header=None)

threads = [2, 4, 8, 16, 32]
K = [3, 5, 7, 9, 11, 13, 15]

processed = pd.DataFrame(np.zeros((len(K), len(threads))), columns=threads, index=K)
processed.iloc[1, 2] = 5
data.shape[0]

k = 0
for i in range(len(K)):
  for j in range(len(threads)):
    if k < data.shape[0]:
      processed.iloc[i, j] = data.iloc[k][0]
    k += 1

processed = processed.transpose()
pd.DataFrame.plot(processed, 'time', '')
processed.plot(figsize=(16,10))