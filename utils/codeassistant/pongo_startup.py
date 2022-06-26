import os
import subprocess
from git import Repo

rootdir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pongodir = rootdir + "/.cached/pongoOS"

if not os.path.exists(pongodir):
    url = "https://github.com/opuntiaOS-Project/pongoOS.git"
    print("Cloning pongoOS to ", pongodir)
    Repo.clone_from(url, pongodir)


def run_command(cmd, cwd="."):
    result = subprocess.run(
        cmd, stdout=subprocess.PIPE, shell=True, cwd=cwd, env=os.environ.copy())
    return (result.stdout.decode("ascii"), result.returncode)


print("Rebuilding pongoOS")
run_command("make -j16", pongodir)

print("Ready: pongoOS")
checkrain_exec = os.environ.get('CHECKRAIN')
if checkrain_exec is None:
    print("Error: No provided $CHECKRAIN env variable.")
    print("Please get a checkrain binary at http://checkra.in/ and set $CHECKRAIN to the binary path.")
    print("")
    print("E.g on macOS after getting binary and installing it to /Application, run:")
    print("\texport CHECKRAIN=/Applications/checkra1n.app/Contents/MacOS/checkra1n")
    exit(1)

print("Checkrain is found. Connect your device and switch it to DFU mode.")
run_command("$CHECKRAIN -k " + pongodir +
            "/build/PongoConsolidated.bin -cpE", pongodir)

opuntiaos_outdir = os.path.dirname(rootdir) + "/out/"
pathrun = "python3 scripts/load_opuntiaos.py -k {0}/rawImage.bin -r {0}/one.img".format(
    opuntiaos_outdir)
run_command(pathrun, pongodir)
