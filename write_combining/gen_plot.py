import pandas as pd
import matplotlib.pyplot as plt

import subprocess
import sys

def run_a_test(num):
    defs = " -DBYTES={} ".format(num)
    cmd_make = 'rm -f ../bin/write_combine && USER_DEFINES="{}" make'.format(defs)
    subprocess.check_output(cmd_make, shell=True)

    return float(subprocess.check_output('../bin/write_combine'))

x_bytes = [16, 32, 48, 64]
y_times = [run_a_test(b) for b in x_bytes]
x_names = [str(x) + ' bytes' for x in x_bytes]

plt.bar(x_names, y_times)
plt.ylabel('Number of cycles to write array')

plt.legend()

plt.savefig('bar.png')
