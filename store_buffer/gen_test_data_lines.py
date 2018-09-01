"""Times the loop with each lines and nt_lines from 0 to 100"""
import subprocess
import numpy as np

def run_a_test(lines, nt_lines, lines_cache, nt_lines_cache):
    defs = " -DLINES={} -DNT_LINES={}".format(lines, nt_lines)
    if lines_cache:
        defs += " -DLINES_IN_CACHE "
    if nt_lines_cache:
        defs += " -DNT_LINES_IN_CACHE "
    cmd_make = 'rm -f ../bin/nt_store_test && USER_DEFINES="{}" make'.format(defs)
    subprocess.check_output(cmd_make, shell=True)

    return float(subprocess.check_output('../bin/nt_store_test'))

vals = []

print "s,nt,time"

for nt in range(0, 52, 2):
    for s in range(0, 52, 2):
        print("{},{},{}".format(s, nt, run_a_test(s, nt, 1, 0)))


