import subprocess
import sys
import os
from tabulate import tabulate
from termcolor import colored
import signal

target_arch = sys.argv[1]

process = subprocess.Popen(
    "./bench.sh", stdout=subprocess.PIPE, preexec_fn=os.setpgrp)
string = ""

sum_of_benchs = {}
count_of_benchs = {}

# For github CI
expected_benchmark_results = {
    "x86": {
        "FORK": 320000,
        "PNG LOADER": 1180000
    },
    "aarch32": {
        "FORK": 954667,
        "PNG LOADER": 5176000
    },
}


def print_results():
    print(colored("Bench results:", color="white", attrs=["bold"]))
    res=[]
    mper=0.0
    for key, value in sum_of_benchs.items():
        new_val=int(value / count_of_benchs[key])
        percent=(1 - new_val /
                   expected_benchmark_results[target_arch][key]) * 100
        res.append([key, expected_benchmark_results[target_arch][key],
                    new_val, "{:.2f}%".format(percent)])
        mper=min(mper, percent)

    data=tabulate(
        res, headers=['Test', 'Expected ({0})'.format(target_arch), 'Got', 'Diff'], tablefmt='orgtbl')
    print(data)
    if (mper < -50):
        print(colored("Crashing: too big performance drop ({0}%)!!!".format(mper), color="red", attrs=["bold"]))
        exit(1)


def process_string(string):
    if (string.startswith("[BENCH]")):
        end_of_date=string.find(" (usec)")
        start_of_date=string.find("] ") + 2
        pr=string[8:start_of_date - 2]
        it=int(string[start_of_date:end_of_date])
        if count_of_benchs.get(pr, None) is None:
            sum_of_benchs[pr]=0
            count_of_benchs[pr]=0

        sum_of_benchs[pr] += it
        count_of_benchs[pr] += 1

    if (string.startswith("[BENCH END]")):
        os.killpg(os.getpgid(process.pid), signal.SIGTERM)
        print_results()


for c in iter(lambda: process.stdout.read(1), b''):
    letter=c.decode()
    if letter == "\n":
        process_string(string)
        string=""
    else:
        string += letter
