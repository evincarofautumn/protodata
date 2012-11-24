#!/bin/bash

here="$(cd "$(dirname "$0")" && pwd)"

if [ "$#" -lt 1 ]; then
  echo "Usage: run.sh /path/to/pd" >&2
  exit 1
fi

PD="$1"
shift

function run_test {

  set +e +E

  test_file="$1"
  test_name="$(basename "$test_file" ".pd")"
  actual_out="$here/$test_name.out.actual"
  expect_out="$here/$test_name.out.expect"
  actual_err="$here/$test_name.err.actual"
  expect_err="$here/$test_name.err.expect"

  if [ ! -e "$expect_err" ]; then
    expect_err=/dev/null
  fi

  $PD "$test_file" > "$actual_out" 2> "$actual_err"

  if [ ! -e "$expect_out" ]; then
    echo "Test '$test_name' BROKEN." >&2
    echo "Expected positive test output ($expect_out) not found." >&2
    exit 1
  fi

  diff -q "$actual_out" "$expect_out"
  if [ "$?" != 0 ]; then
    echo "Test '$test_name' FAILED." >&2
    echo "Positive test output does not match expected." >&2
    exit 1
  fi

  diff -u "$actual_err" "$expect_err"
  if [ "$?" != 0 ]; then
    echo "Test '$test_name' FAILED." >&2
    echo "Negative test output does not match expected." >&2
    echo
    echo "Expected:" >&2
    cat "$expect_err" >&2
    echo
    echo "Actual:" >&2
    cat "$actual_err" >&2
    echo
    exit 1
  fi

  echo "Test '$test_name' passed."

  set -e -E

}

find "$here" -maxdepth 1 -name '*.pd' | while read test_file; do
  run_test "$test_file"
done
