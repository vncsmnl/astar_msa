#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

"${SCRIPT_DIR}/performance_xeon_1aboA.sh"
"${SCRIPT_DIR}/performance_xeon_actin.sh"
"${SCRIPT_DIR}/performance_xeon_2ack.sh"
"${SCRIPT_DIR}/performance_xeon_2cba.sh"
"${SCRIPT_DIR}/performance_xeon_1sesA.sh"
