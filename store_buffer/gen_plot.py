import sys

import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

mycsv = pd.read_csv(sys.argv[1], header=0)
indexed = mycsv.pivot('nt', 's', 'time')

ax = sns.heatmap(indexed, xticklabels=2, yticklabels=2, cmap=sns.color_palette("RdGy", 100))

ax.invert_yaxis()

plt.xlabel("Number of cache lines written normally")
plt.ylabel("Number of cache lines written nontemporally")
plt.title("Cycles spent to run nontemporal stores and then temporal stores with sfence between")

plt.savefig(sys.argv[1] + '.png')
