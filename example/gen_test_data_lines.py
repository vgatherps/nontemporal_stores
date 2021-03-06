import subprocess
import numpy as np

def run_a_test(num, do_nt):
    defs = ""
    if do_nt:
        defs += " -DNT_COPY -DBUF=50 -DN_IDS={}".format(num)
    else:
        defs += " -DN_IDS={} -DBUF=50 ".format(num)
    cmd_make = 'rm -f ../bin/example && USER_DEFINES="{}" make'.format(defs)
    subprocess.check_output(cmd_make, shell=True)

    return subprocess.check_output('../bin/example').strip('\n\r')

ids_to_time = [10, 100, 1000, 10000, 100000, 1000000]

print "s,time,cycle,timent,cyclent"

for n in ids_to_time:
    print("{},{},{}".format(n, run_a_test(n, False), run_a_test(n, True)))


