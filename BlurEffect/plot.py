import matplotlib.pyplot as plt

f = open('out.out')
t = f.readlines()
f.close()

t = [float(i.strip('\n')) for i in t]
thr = [2**i for i in range(len(t))]

plt.plot(thr, t)
plt.xlabel('time')
plt.ylabel('threads')
plt.savefig('performance.png')
