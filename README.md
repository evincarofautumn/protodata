# Protodata

**Protodata** is a language for describing binary data,
originally written for prototyping binary file formats.
After Unix tradition, it’s designed to be lightweight and
composable, and interact well with other utilities.

# Building

```
git clone git@github.com:evincarofautumn/protodata.git
cd protodata
make
```

Protodata has been tested with G++ 4.6 on Linux.

# Examples

For the purposes of illustration, assume the following alias:

```
alias dump="hexdump -v -e '/1 \"%02X \"'"
```

View the hex representation of a floating-point number:

```
$ pd -e 'f64 1.0' | dump; echo
```

View the UTF-16 representation of some UTF-8 text:

```
$ pd -e 'utf16 "blah"' | dump; echo
```

Check out what a big-endian floating-point negative infinity
looks like up close:

```
$ pd -e 'big f64 -inf' | dump; echo
```

----

Suppose you have some raw vertex data in `input.raw`:

```
+1.0 +1.0 -1.0
+1.0 -1.0 -1.0
-1.0 -1.0 -1.0
-1.0 +1.0 -1.0
+1.0 +1.0 +1.0
-1.0 +1.0 +1.0
-1.0 -1.0 +1.0
+1.0 -1.0 +1.0
```

You want to pack these values as single-precision floats
into `output.bin` in such a way that you can `mmap()` the
output and index it as an array. If you didn’t have
Protodata, you might write a one-off C program like this:

```
cat <<EOF | gcc -x c - && ./a.out
#include <stdio.h>
int main(int argc, char** argv) {
  FILE* in = fopen("input.raw", "r");
  FILE* out = fopen("output.bin", "w");
  float value;
  while (fscanf(in, " %f", &value) == 1)
    fwrite(&value, sizeof(value), 1, out);
  return 0;
}
EOF
```

To do the same thing with Protodata is straightforward:

```
$ pd -e 'f32' input.raw -o output.bin
```

Here, `f32` is a Protodata command, which sets the output
mode to 32-bit `float`. Now, both the C version and the
Protodata version use platform endianness. If we wanted to
change that to big-endian in the C version, we would have to
detect endianness and manually swap byte order. In
Protodata, however, we can simply use the `big` command:

```
$ pd -e 'big f32' input.raw -o output.bin
```

# Command-line Options

 * `FILE`

   Read and evaluate `FILE` as Protodata source.

 * `-e`, `--eval`

   Evaluate a string as Protodata source.

 * `-o`, `--output`

   Specify an output file; default is standard output.

Multiple `-e` and `FILE` options may be specified; they are
all concatenated in the order they appeared on the command
line, *before* being evaluated and *after* being parsed.
This means that any *individual* source file or `-e` string
is allowed to contain semantically invalid Protodata source,
as long as the concatenated source is semantically valid.

# The Language

*Features marked with a dagger (†) are subject to change.*

A Protodata source file consists of a series of *values*
and *commands*, delimited by whitespace and punctuated by
comments. Comments begin with `#` and continue to the end of
the line:

```
# A useless comment for demonstration purposes.
```

Source files must be encoded in UTF-8.

## Values

### Integer

#### Examples

```
 0      # Decimal, technically.
-12     # Decimal in the usual sense.
 020    # Still decimal.
+0b1010 # Binary.
-0o777  # Octal.
+0xFE   # Hexadecimal.
```

#### Regular Expression

```
[+-]?(0(b[01_]+|o[0-7_]+|x[0-9A-Fa-f_]+|[0-9_]*))
```

#### Notes

Integers with no sign character (`+` or `-`) are interpreted
as unsigned—go figure—and must fit within the range of
`uint64_t`; signed integers must fit within the range of
`int64_t`.

### Floating-point

#### Examples

```
 0.0
-0.0
 0.5
+0.5
 6.28318
 inf
-inf
 nan
 epsilon
```

#### Regular Expression

```
[+-]?([0-9][0-9_]*\.[0-9][0-9_]*|inf|nan|epsilon)
```

#### Notes

Floating-point values cannot be represented by `utf` or
integer types. Unlike for integers, sign characters have no
effect on floating-point representation, only sign bit.

### String

#### Examples

```
""
"salut"
```

#### Regular Expression

```
"([^"\\]|\\[\\"abefnrtv])*"
```

#### Notes

A string is syntactic sugar for a sequence of integer values
corresponding to Unicode code points. Strings are not
null-terminated; an empty string is a no-op. The following
common escapes are allowed:

 * `\\` ⇒ `\` = U+005C

 * `\"` ⇒ `"` = U+0022

 * `\a` ⇒ BEL = U+0007

 * `\b` ⇒ BS = U+0008

 * `\e` ⇒ ESC = U+001B

 * `\f` ⇒ FF = U+000C

 * `\n` ⇒ LF = U+000A

 * `\r` ⇒ CR = U+000D

 * `\t` ⇒ HT = U+0009

 * `\v` ⇒ VT = U+000B

## Commands

### Size and Encoding

 * `s8`, `s16`, `s32`, `s64`

   A signed integer of the given bit width.

 * `u8`, `u16`, `u32`, `u64`

   An unsigned integer of the given bit width.

 * `f32`, `f64`

   A single- or double-precision floating-point number.

 * `utf8`, `utf16`, `utf32`

   A code point in UTF-8, UTF-16, or UTF-32. The following
   produces three copies of the same (two-byte) sequence:

        u8 0xC3 0xA7
        utf8 "ç"
        utf8 231

The default type is `unsigned int`, however wide `int`
happens to be on the current platform.†

### Endianness

 * `native`

 * `little`

 * `big`

The default endianness is `native`. The `s8`, `u8`, and
`utf8` types ignore the current endianness.

### Compiler State

 * `{`

   Save the compiler state.

 * `}`

   Restore the compiler state.

These commands are best explained by example. Here, `10`,
`20`, and `30` will be written as `u32`, while `1` through
`6` will all be written as `u8`:

```
u32
10 { u8 1 }
20 { u8 2 3 }
30 { u8 4 5 6 }
```

There is no requirement that braces must be balanced.†

# Planned Features

## Macros

```
name(...) = { ... }
```

## Expressions

```
( ... )
```

Evaluate an expression with `+`, `-`, `*`, `/`, and `%`
operators, as well as the following sorts of functions:

 * `align(N = 1, X = 0)`

   Align subsequent values on `N`-byte boundaries with
   `u8` padding value `X`.

 * `repeat(N = 1)`

   Repeat the next value `N` times.

 * `count(Name)`

   Write the number of elements in `Name`, where all of the
   elements must have the same type. Useful for Pascal-style
   strings:

        pascal(string) = { count(string) string() }

 * `size(Expr)`

   Write the size in bytes of `Expr`.
