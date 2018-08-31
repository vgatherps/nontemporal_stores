import pandas as pd
import matplotlib.pyplot as plt

import sys

my_csv = pd.read_csv(sys.argv[1], header=0)
my_csv['nontemporal'] = my_csv['timent']
my_csv['normal'] = my_csv['time']

plt.plot('s', 'nontemporal', data=my_csv)
plt.plot('s', 'normal', data=my_csv)

plt.xlabel('Number of cache lines written')
plt.ylabel('Number of cycles to reload variable')

plt.title('Cycles to re-load variable vs number of cache lines stored')

plt.legend()

plt.savefig(sys.argv[1] + '.png')
