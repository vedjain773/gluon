#!/usr/bin/env bash
#
# Runs every tests/*.c through the gluon compiler (unoptimized and optimized),
# assembles/links for RISC-V, runs the result and compares the exit code with
# the "EXPECTED: <n>" comment found in the test file.
#
# Usage: ./check_tests.sh [-v] [-t SECONDS] [test_dir_or_files...]
#   -v   verbose: show compiler/assembler/linker output on failure
#   -t   per-test run timeout in seconds (default: 5)
#
# Exit status: 0 if all tests pass, 1 otherwise.

set -u

GLUON=./gluon
AS=riscv64-linux-gnu-as
CC=riscv64-linux-gnu-gcc
TIMEOUT=5
VERBOSE=0

while getopts "vt:" opt; do
    case $opt in
        v) VERBOSE=1 ;;
        t) TIMEOUT=$OPTARG ;;
        *) echo "Usage: $0 [-v] [-t SECONDS] [files...]" >&2; exit 2 ;;
    esac
done
shift $((OPTIND - 1))

if [ $# -gt 0 ]; then
    TESTS=("$@")
else
    TESTS=(tests/*.c)
fi

if [ -t 1 ]; then
    BLUE=$'\033[34m'; RED=$'\033[31m'; GREEN=$'\033[32m'; RESET=$'\033[0m'; CLEAR=$'\r\033[K'
else
    BLUE=''; RED=''; GREEN=''; RESET=''; CLEAR=''
fi

for tool in "$GLUON" "$AS" "$CC"; do
    if ! command -v "$tool" >/dev/null 2>&1 && [ ! -x "$tool" ]; then
        echo "${RED}error:${RESET} required tool not found: $tool" >&2
        exit 2
    fi
done
if [ ! -e "${TESTS[0]}" ]; then
    echo "${RED}error:${RESET} no test files found" >&2
    exit 2
fi

RUNNER=()
if [ "$(uname -m)" != "riscv64" ]; then
    if command -v qemu-riscv64 >/dev/null 2>&1; then
        RUNNER=(qemu-riscv64)
    fi
fi

WORKDIR=$(mktemp -d)
trap 'rm -rf "$WORKDIR"' EXIT

quiet() {
    if [ "$VERBOSE" -eq 1 ]; then "$@"; else "$@" >/dev/null 2>&1; fi
}

run_suite() {
    local title=$1; shift
    local flags=("$@")
    SUITE_PASSED=0
    SUITE_TOTAL=0

    echo "${BLUE}[$title]${RESET}"

    local file expected actual
    for file in "${TESTS[@]}"; do
        printf '%s%s[Testing]%s %s' "$CLEAR" "$BLUE" "$RESET" "$file"
        SUITE_TOTAL=$((SUITE_TOTAL + 1))

        expected=$(grep -m1 "EXPECTED:" "$file" | cut -d':' -f2 | xargs)
        if ! [[ $expected =~ ^[0-9]+$ ]]; then
            printf '%s%s[FAIL]%s %s: missing or invalid EXPECTED: line\n' "$CLEAR" "$RED" "$RESET" "$file"
            continue
        fi

        rm -f "$WORKDIR"/prog*

        if ! quiet "$GLUON" "$file" "${flags[@]}" -o "$WORKDIR/prog.s"; then
            printf '%s%s[FAIL]%s %s: compiler error\n' "$CLEAR" "$RED" "$RESET" "$file"
            continue
        fi
        if ! quiet "$AS" "$WORKDIR/prog.s" -o "$WORKDIR/prog.o"; then
            printf '%s%s[FAIL]%s %s: assembler error\n' "$CLEAR" "$RED" "$RESET" "$file"
            continue
        fi
        if ! quiet "$CC" -static "$WORKDIR/prog.o" -o "$WORKDIR/prog"; then
            printf '%s%s[FAIL]%s %s: linker error\n' "$CLEAR" "$RED" "$RESET" "$file"
            continue
        fi

        timeout "$TIMEOUT" "${RUNNER[@]}" "$WORKDIR/prog" >/dev/null 2>&1
        actual=$?

        if [ "$actual" -eq 124 ]; then
            printf '%s%s[FAIL]%s %s: timed out after %ss\n' "$CLEAR" "$RED" "$RESET" "$file" "$TIMEOUT"
        elif [ "$actual" -eq "$expected" ]; then
            SUITE_PASSED=$((SUITE_PASSED + 1))
        else
            printf '%s%s[FAIL]%s %s: expected=%s got=%s\n' "$CLEAR" "$RED" "$RESET" "$file" "$expected" "$actual"
        fi
    done

    local color=$GREEN
    [ "$SUITE_PASSED" -ne "$SUITE_TOTAL" ] && color=$RED
    printf '%sTotal tests passed: %s%s / %s%s\n' "$CLEAR" "$color" "$SUITE_PASSED" "$SUITE_TOTAL" "$RESET"
}

overall_ok=0

run_suite "Testing Unoptimized IR"
[ "$SUITE_PASSED" -ne "$SUITE_TOTAL" ] && overall_ok=1

echo
run_suite "Testing Optimized IR" --optimize
[ "$SUITE_PASSED" -ne "$SUITE_TOTAL" ] && overall_ok=1

exit $overall_ok
