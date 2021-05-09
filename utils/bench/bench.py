import subprocess
import sys
import os
from tabulate import tabulate
import signal

process = subprocess.Popen(
    "./bench.sh", stdout=subprocess.PIPE, preexec_fn=os.setpgrp)
string = ""

sum_of_benchs = {}
count_of_benchs = {}

# For github CI
expected_benchmark_results = {
    "FORK": 320000,
    "PNG LOADER": 1180000
}


def print_results():
    print("Bench results:")
    res = []
    mper = 0.0
    for key, value in sum_of_benchs.items():
        new_val = int(value / count_of_benchs[key])
        percent = (1 - new_val / expected_benchmark_results[key]) * 100
        res.append([key, expected_benchmark_results[key],
                    new_val, "{:.2f}%".format(percent)])
        mper = min(mper, percent)

    data = tabulate(
        res, headers=['Test', 'Expected', 'Got', 'Diff'], tablefmt='orgtbl')
    print(data)
    if (mper < -50):
        exit(1)


def process_string(string):
    if (string.startswith("[BENCH]")):
        end_of_date = string.find(" (usec)")
        start_of_date = string.find("] ") + 2
        pr = string[8:start_of_date - 2]
        it = int(string[start_of_date:end_of_date])
        if count_of_benchs.get(pr, None) is None:
            sum_of_benchs[pr] = 0
            count_of_benchs[pr] = 0

        sum_of_benchs[pr] += it
        count_of_benchs[pr] += 1

    if (string.startswith("[BENCH END]")):
        os.killpg(os.getpgid(process.pid), signal.SIGTERM)
        print_results()


for c in iter(lambda: process.stdout.read(1), b''):
    letter = c.decode()
    if letter == "\n":
        process_string(string)
        string = ""
    else:
        string += letter
