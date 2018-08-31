"""Times the loop with each lines and nt_lines from 0 to 100"""
import subprocess
import numpy as np

def run_a_test(num, do_nt):
    defs = ""
    if do_nt:
        defs += " -DNT_STORES={}".format(s)
    else:
        defs += " -DSTORES={}".format(s)
    cmd_make = 'rm -f ../bin/write_allocate && USER_DEFINES="{}" make'.format(defs)
    subprocess.check_output(cmd_make, shell=True)

    return float(subprocess.check_output('../bin/write_allocate'))

vals = []

print "s,time,timent"

for s in range(0, 16348+64, 64):
    print("{},{},{}".format(s, run_a_test(s, False), run_a_test(s, True)))


