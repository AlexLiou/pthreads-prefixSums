#!/usr/bin/env python3
import os
from subprocess import check_output
import re
from time import sleep

#
#  Feel free (a.k.a. you have to) to modify this to instrument your code
#

THREADS = [0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32]
LOOPS = [20]
INPUTS = ["1k.txt", "8k.txt", "16k.txt"]

csvs = []
for inp in INPUTS:
    for loop in LOOPS:
        csv = ["{}/{}".format(inp, loop)]
        for thr in THREADS:
            cmd = "./bin/prefix_scan -o temp.txt -n {} -i tests/{} -l {}".format(
                thr, inp, loop)
            out = check_output(cmd, shell=True).decode("ascii")
            m = re.search("time: (.*)", out)
            if m is not None:
                time = m.group(1)
                csv.append(time)

        csvs.append(csv)
        sleep(0.5)

header = ["microseconds"] + [str(x) for x in THREADS]

print("\n")
print(", ".join(header))
for csv in csvs:
    print (", ".join(csv))


# # Assignment Constants
# FIGSIZE = (12,8)
# plt.figure(figsize=FIGSIZE)
# plt.plot(THREADS, csvs, label='', marker='o')
# plt.xlabel('Number of Threads')
# plt.ylabel('Execution Time')
# plt.title('Number of Threads vs Execution Time.')
# plt.legend()
# plt.grid(True)
# plt.show()