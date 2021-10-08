import subprocess
import sys
import os
from tabulate import tabulate
from termcolor import colored
import signal

target_arch = sys.argv[1]

process = subprocess.Popen(
    "./run_tester.sh", stdout=subprocess.PIPE, preexec_fn=os.setpgrp)
string = ""

def process_string(string):
    
    if (string.startswith("[OK]")):
        print(string.replace("$", "/"))

    if (string.startswith("[MSG]")):
        print(string[4:])

    if (string.startswith("[ALL TESTS PASSED]")):
        print(string)
        os.killpg(os.getpgid(process.pid), signal.SIGTERM)

    if (string.startswith("[FAILED]")):
        print(string)
        os.killpg(os.getpgid(process.pid), signal.SIGTERM)
        exit(1)


for c in iter(lambda: process.stdout.read(1), b''):
    letter=c.decode()
    if letter == "\n":
        process_string(string)
        string=""
    else:
        string += letter
