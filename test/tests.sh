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

export HONCHO_DIR=$(mktemp -d)/
DEFAULT_QUEUE=$HONCHO_DIR/queue/default; mkdir -p $DEFAULT_QUEUE
FOOBAR_QUEUE=$HONCHO_DIR/queue/foobar; mkdir -p $FOOBAR_QUEUE

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
	rm -r $HONCHO_DIR
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
		$HONCHO execute -w test-01 echo ping > /dev/null
		test -d $DEFAULT_QUEUE/test-01
		test_okfail $?

	test_title "file return_code is zero"
		RC=$(cat $DEFAULT_QUEUE/test-01/return_code)
		test "x$RC" = "x0"
		test_okfail $?

	test_title "file command is correct"
		CMD=$(cat $DEFAULT_QUEUE/test-01/command)
		test "x$CMD" = "xecho ping"
		test_okfail $?

	test_title "file stdout is correct"
		STDOUT=$(cat $DEFAULT_QUEUE/test-01/stdout)
		test "x$STDOUT" = "x$(echo "0: ping")"
		test_okfail $?

	test_title "file stderr is empty"
		STDERR=$(cat $DEFAULT_QUEUE/test-01/stderr)
		test "x$STDERR" = "x"
		test_okfail $?

	test_title "file status is done"
		cat $DEFAULT_QUEUE/test-01/status | grep "status: done"  >/dev/null
		test_okfail $?

test_section "honcho cat"
	test_title "is able to show stdout"
		$HONCHO cat test-01 stdout >$DEFAULT_QUEUE/test-01/stdout-via-cat
		diff $DEFAULT_QUEUE/test-01/stdout $DEFAULT_QUEUE/test-01/stdout-via-cat >/dev/null
		test_okfail $?

test_section "honcho status"
	test_title "displays status file (!running)"
		$HONCHO status test-01 >$DEFAULT_QUEUE/test-01/status-via-status
		diff $DEFAULT_QUEUE/test-01/status $DEFAULT_QUEUE/test-01/status-via-status  >/dev/null
		test_okfail $?

test_section "honcho submit"
	test_title "generates a pending file"
		FILE=$($HONCHO submit ls)
		test -f $DEFAULT_QUEUE/$FILE.pending
		test_okfail $?

	test_title "file contains command"
		CMD=$(cat $DEFAULT_QUEUE/$FILE.pending)
		test "x$CMD" = "xls"
		test_okfail $?

test_section "multiple queues"
	test_title "works with execute"
		$HONCHO -q foobar execute -w test-02 echo test
		test -d $FOOBAR_QUEUE/test-02 && test -e $FOOBAR_QUEUE/test-02/command
		test_okfail $?

	test_title "works with cat"
		$HONCHO -q foobar cat test-02 stdout >$FOOBAR_QUEUE/test-02/stdout-via-cat
		diff $FOOBAR_QUEUE/test-02/stdout $FOOBAR_QUEUE/test-02/stdout-via-cat >/dev/null
		test_okfail $?
	
	test_title "works with status"
		$HONCHO -q foobar status test-02 >$FOOBAR_QUEUE/test-02/status-via-status
		diff $FOOBAR_QUEUE/test-02/status $FOOBAR_QUEUE/test-02/status-via-status  >/dev/null
		test_okfail $?

	test_title "works with submit"
		FILE=$($HONCHO -q foobar submit ls)
		test -f $FOOBAR_QUEUE/$FILE.pending
		test_okfail $?

test_section "honcho state"
	test_title "defaults to 'online'"
		STATE=$($HONCHO state)
		test "x$STATE" = "xonline"
		test_okfail $?

	test_title "can be changed to 'silly'"
		STATE=$($HONCHO state silly)
		test_okfail $?

	test_title "is stored in data directory"
		test -e $HONCHO_DIR/state
		test_okfail $?

test_teardown

expr $TEST_FAIL_COUNT = 0 >/dev/null || exit

test_section "status"
	test_title "Test OK"; $ECHO $TEST_OK_COUNT
	test_title "Test FAIL"; $ECHO $TEST_FAIL_COUNT
