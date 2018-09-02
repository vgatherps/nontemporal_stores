import subprocess
import numpy as np

def run_a_test(num, buf, do_nt):
    defs = ""
    if do_nt:
        defs += " -DNT_COPY -DN_IDS={} -DBUF={}".format(num, buf)
    else:
        defs += " -DN_IDS={} -DBUF={}".format(num, buf)
    cmd_make = 'rm -f ../bin/example && USER_DEFINES="{}" make'.format(defs)
    subprocess.check_output(cmd_make, shell=True)

    return subprocess.check_output('../bin/example').strip('\r\n')

ids_to_time = [1000, 10000, 100000, 1000000]
buf_mb = [1, 10, 20, 30, 50, 100, 200]

print "s,b,time,cycle,timent,cyclent"

for n in ids_to_time:
    for b in buf_mb:
        print("{},{},{},{}".format(n, b, run_a_test(n, b, False), run_a_test(n, b, True)))
