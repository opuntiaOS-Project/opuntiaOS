import subprocess
import sys
import os
from termcolor import colored
import signal

# verbose
flag = ""
if len(sys.argv) >= 2:
    flag = sys.argv[1]

process = subprocess.Popen(
    "./run_tester.sh", stdout=subprocess.PIPE, preexec_fn=os.setpgrp)
string = ""


def process_string(string):

    if flag == "verbose":
        print(string)

    if (string.startswith("[OK]")):
        print(colored("ok ", color="green", attrs=[
              "bold"]), string.replace("$", "/")[5:])

    if (string.startswith("[MSG]")):
        print(string[6:])

    if (string.startswith("[ALL TESTS PASSED]")):
        print(colored(string, color="green", attrs=["bold"]))
        os.killpg(os.getpgid(process.pid), signal.SIGTERM)

    if (string.startswith("[FAILED]")):
        print(colored("failed ", color="red", attrs=[
              "bold"]), string.replace("$", "/")[9:])
        os.killpg(os.getpgid(process.pid), signal.SIGTERM)
        exit(1)


for c in iter(lambda: process.stdout.read(1), b''):
    letter = c.decode()
    if letter == "\n":
        process_string(string)
        string = ""
    else:
        string += letter
