# Run from the root dir.

SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]:-$0}"; )" &> /dev/null && pwd 2> /dev/null; )";
$SCRIPT_DIR/x86.sh
$SCRIPT_DIR/x86_64.sh
$SCRIPT_DIR/arm.sh
$SCRIPT_DIR/arm64.sh
$SCRIPT_DIR/riscv64.sh
