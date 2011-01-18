#/bin/bash

TESTNUM=0
TEST_OK_COUNT=0
TEST_FAIL_COUNT=0

#COLORS
C_RESET="\033[0m"
C_PURPLE="\033[35m"
C_YELLOW="\033[33m"
C_GREEN="\033[32m"
C_RED="\033[31m"

ECHO=echo
UNAME=`uname`
R=0 # return value
HONCHO=../src/honcho
export HONCHO_QUEUE_DIR=$(mktemp -d)/
export TZ="Europe/Copenhagen"

test_section() {
	local title="$1" # IN

	$ECHO "\n${C_PURPLE}** $title${C_RESET}\n"
}

test_title() {
	local title="$1" # IN
	local len=`$ECHO $title|wc -c`
	local dots
	local str

	TESTNUM=`expr $TESTNUM + 1`

	dots=`expr 30 - $len`
	str=""
	while test $dots -gt 1 ; do
		dots=`expr $dots - 1`
		str=".$str"
	done

	printf "  ${C_YELLOW}t%02d${C_RESET} ${title}${str}: " $TESTNUM
}

test_teardown() {
	rm -r $HONCHO_QUEUE_DIR
}

test_okfail() {
	local rc=$1 # IN
	if [ $rc -eq 0 ]; then
		$ECHO "${C_GREEN}OK${C_RESET}"
		TEST_OK_COUNT=`expr $TEST_OK_COUNT + 1`
	else
		$ECHO "${C_RED}FAIL${C_RESET}"
		TEST_FAIL_COUNT=`expr $TEST_FAIL_COUNT + 1`
	fi
}

test_failok() {
	local rc=$1 # IN
	if [ $rc -eq 0 ]; then
		$ECHO "${C_RED}FAIL${C_RESET}"
		TEST_FAIL_COUNT=`expr $TEST_FAIL_COUNT + 1`
	else
		$ECHO "${C_GREEN}OK${C_RESET}"
		TEST_OK_COUNT=`expr $TEST_OK_COUNT + 1`
	fi
}

## 

if test ! -f $HONCHO ; then
	$ECHO "$HONCHO not found, aborting"
	exit
fi

test_section "honcho execute"
	test_title "creates new directory"
		$HONCHO execute test-01 echo ping > /dev/null
		test -d $HONCHO_QUEUE_DIR/test-01
		test_okfail $?

	test_title "file return_code is zero"
		RC=$(cat $HONCHO_QUEUE_DIR/test-01/return_code)
		test "x$RC" = "x0"
		test_okfail $?

	test_title "file command is correct"
		CMD=$(cat $HONCHO_QUEUE_DIR/test-01/command)
		test "x$CMD" = "xecho ping"
		test_okfail $?

	test_title "file stdout is correct"
		STDOUT=$(cat $HONCHO_QUEUE_DIR/test-01/stdout)
		test "x$STDOUT" = $(echo "xping\n")
		test_okfail $?

	test_title "file stderr is empty"
		STDERR=$(cat $HONCHO_QUEUE_DIR/test-01/stderr)
		test "x$STDERR" = "x"
		test_okfail $?

	test_title "file status is done"
		cat $HONCHO_QUEUE_DIR/test-01/status | grep "status: done"  >/dev/null
		test_okfail $?

test_section "honcho cat"
	test_title "is able to show stdout"
		$HONCHO cat test-01 stdout >$HONCHO_QUEUE_DIR/test-01/stdout-via-cat
		diff $HONCHO_QUEUE_DIR/test-01/stdout $HONCHO_QUEUE_DIR/test-01/stdout-via-cat >/dev/null
		test_okfail $?

test_section "honcho status"
	test_title "displays status file (!running)"
		$HONCHO status test-01 >$HONCHO_QUEUE_DIR/test-01/status-via-status
		diff $HONCHO_QUEUE_DIR/test-01/status $HONCHO_QUEUE_DIR/test-01/status-via-status  >/dev/null
		test_okfail $?

test_teardown

expr $TEST_FAIL_COUNT = 0 >/dev/null || exit

test_section "status"
	test_title "Test OK"; $ECHO $TEST_OK_COUNT
	test_title "Test FAIL"; $ECHO $TEST_FAIL_COUNT
