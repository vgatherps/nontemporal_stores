import sys

import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

mycsv = pd.read_csv(sys.argv[1], header=0)
mycsv['diff'] = 100 * (mycsv['timent'] - mycsv['time']) / mycsv['time']
indexed_t = mycsv.pivot('b', 's', 'diff')

ax = sns.heatmap(indexed_t, robust=True, center=0)

ax.invert_yaxis()

plt.xlabel("Number of elements in lookup map")
plt.ylabel("Size of storage buffer in MB")
plt.title("% difference, total runtime normal and nontemporal stores")

plt.savefig(sys.argv[1] + '.png')

plt.clf()

mycsv['diff'] = 100 * (mycsv['cyclent'] - mycsv['cycle']) / mycsv['cycle']
indexed_t = mycsv.pivot('b', 's', 'diff')

ax = sns.heatmap(indexed_t, robust=True, center=0)

ax.invert_yaxis()

plt.xlabel("Number of elements in lookup map")
plt.ylabel("Size of storage buffer in MB")
plt.title("% difference, lookup cycles for normal and nontemporal stores")

plt.savefig(sys.argv[1] + '.nt.png')
