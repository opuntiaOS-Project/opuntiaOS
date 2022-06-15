# Run from the root dir.

SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]:-$0}"; )" &> /dev/null && pwd 2> /dev/null; )";
$SCRIPT_DIR/x86.sh
$SCRIPT_DIR/aarch64.sh
$SCRIPT_DIR/arm.sh
