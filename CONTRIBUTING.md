# Contributing to Protodata

## Write a Test

 1. Add a test document in
    <code>test/<var>name</var>.pd</code>

 2. Add the expected binary output in
    <code>test/<var>name</var>.out.expect</code>

 3. Add the expected error output, if any, in
    <code>test/<var>name</var>.err.expect</code>

 4. Run the entire test suite with `make test` or just your
    test with <code>make test-<var>name</var></code>.

 5. Submit a pull request.

## Report a Bug

 1. Write a failing test case that demonstrates the bug.

 2. Submit a pull request.

**Or:**

 1. Add an [issue][1] with a complete description.

## Add a Feature

 1. Write failing test cases—if your code is removed, the
    test suite should fail!

 2. Write code, following the style guide below.

 3. Rebase your commits if necessary so they’re easier to
    review. Be explicit when committing broken or failing
    code, e.g.:

    ```
    Add failing test for frobnication.
    ```

 4. Submit a pull request.

## Style Guide

Protodata is written in C++11 using two-space indentation
and One True Brace Style. There is a general preference for
functions over classes, STL algorithms over explicit loops,
and `const` over non-`const` variables. Macros are okay, but
remember to `#undef` them or prefer templates where
possible. Classes are in `CamelCase`, functions and
variables in `underscore_case()`, and macros in
`SHOUTY_CASE`.

Headers are always included with `<...>`, not `"..."`. More
specific headers should be included before less specific
ones, in an effort to avoid implicit dependencies. Headers
with the same specificity are grouped and sorted
lexicographically. Forward-declare things when you can.

Comment difficult code if you have to. And avoid writing
difficult code in the first place!

[1]: https://github.com/evincarofautumn/protodata/issues
