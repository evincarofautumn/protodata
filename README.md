# Protodata

**Protodata** is a language for describing binary data, originally written for prototyping and reverse engineering binary file formats. It is particularly useful for generating annotated data in a custom format, *without* having to write custom editing tools.

After Unix tradition, Protodata is designed to be lightweight and composable, and interact well with other utilities.

# Building

```
git clone git://github.com/evincarofautumn/protodata.git
cd protodata
make
./pd --help
```

Protodata has been tested with G++ 4.6 and Clang 3.2 on Linux.

# Examples

For the purposes of illustration, assume the following alias:

```
alias dump="hexdump -v -e '/1 \"%02X \"'"
```

## Floating-point

View the representation of a floating-point number:

```
$ pd -e 'f64 1.0' | dump; echo

$ pd -e 'big f64 -inf' | dump; echo
```

## Unicode

Convert from UTF-8 to different transmission formats:

```
$ pd -e 'utf16 "Some UTF-8 text I’d like to reëncode."' | dump; echo

$ pd -e 'ucs2 "Because who needs surrogate pairs?"' | dump; echo

$ pd -e 'u21 "Mind the bits and the bytes will look after themselves."' | dump; echo
```

## More!

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

You want to pack these values as single-precision floats into `output.bin` in such a way that you can `mmap()` the output and index it as an array. If you didn’t have Protodata, you might write a one-off C program like this:

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

Here, `f32` is a Protodata command, which sets the output mode to 32-bit `float`. Now, both the C version and the Protodata version use platform endianness. If we wanted to change that to big-endian in the C version, we would have to detect endianness and manually swap byte order. In Protodata, however, we can simply use the `big` command:

```
$ pd -e 'big f32' input.raw -o output.bin
```

Like the C version, Protodata consumes input lazily and produces output eagerly, making it suitable for streaming:

```
$ cat input.raw | pd -e 'f32' - -o output.bin
```

# Command-line Options

 * `-h`, `--help`

   Print a help message and exit.

 * `FILE`

   Read and evaluate `FILE` as Protodata source.

 * `-e`, `--eval`

   Evaluate a string as Protodata source.

 * `-o`, `--output`

   Specify an output file; default is standard output.

Multiple `-e` and `FILE` options may be specified; they are all concatenated in the order they appeared on the command line. This means any *individual* source file or `-e` string is allowed to contain semantically invalid Protodata source, as long as the concatenated source is semantically valid.

# The Language

*Features marked with a dagger (†) are subject to change.*

A Protodata source file consists of a series of *values* and *commands*, delimited by whitespace and punctuated by comments. Comments begin with `#` and continue to the end of the line:

```
# A useless comment for demonstration purposes.
```

**Source files must be encoded in UTF-8.**

## Values

### Integer

```
 0      # Decimal, technically.
-12     # Decimal in the usual sense.
 020    # Still decimal.
+0b1010 # Binary.
-0o777  # Octal.
+0xFE   # Hexadecimal.
```

Integers with a sign character (`+` or `-`) are interpreted as signed values—go figure—and must fit within the range of `int64_t`; unsigned integers are those without a sign character, and must fit within the range of `uint64_t`. Out-of-range values are reported as errors, e.g.:

```
s32 +0xFFFFFFFF
```

### Floating-point

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

Floating-point values cannot be represented by `utf` or integer types. Unlike for integers, sign characters have no effect on floating-point representation, only sign bit.

### String

```
""
"salut"
```

A string is syntactic sugar for a sequence of integer values corresponding to Unicode code points. Strings are not null-terminated; an empty string is a no-op. The following common escapes are allowed:

<table>
<tr><th>Escape</th><th>Character</th><th>Unicode</th></tr>
<tr><td><code>\a</code></td><td>BEL</td><td>U+0007</td></tr>
<tr><td><code>\b</code></td><td>BS</td><td>U+0008</td></tr>
<tr><td><code>\t</code></td><td>HT</td><td>U+0009</td></tr>
<tr><td><code>\n</code></td><td>LF</td><td>U+000A</td></tr>
<tr><td><code>\v</code></td><td>VT</td><td>U+000B</td></tr>
<tr><td><code>\f</code></td><td>FF</td><td>U+000C</td></tr>
<tr><td><code>\r</code></td><td>CR</td><td>U+000D</td></tr>
<tr><td><code>\e</code></td><td>ESC</td><td>U+001B</td></tr>
<tr><td><code>\"</code></td><td><tt>"</tt></td><td>U+0022</td></tr>
<tr><td><code>\\</code></td><td><tt>\</tt></td><td>U+005C</td></tr>
</table>

## Commands

### Size and Encoding

 * <code>s<var>N</var></code>

   A signed integer with <code><var>N</var></code> bits, where 1 ≤ <code><var>N</var></code> ≤ 64.

 * <code>u<var>N</var></code>

   An unsigned integer with <code><var>N</var></code> bits, where 1 ≤ <code><var>N</var></code> ≤ 64.

 * `f32`, `f64`

   A single- or double-precision floating-point number.

 * `utf8`, `utf16`, `utf32`, `ucs2`

   A Unicode code point in the given transmission format. `utf32` is an alias for `u32`, and `ucs2` for `u16`.

   The following produces three copies of the same (two-byte) sequence:

        u8 0xC3 0xA7
        utf8 "ç"
        utf8 231

If the whole output ends on a non-8-bit boundary, it will be padded with trailing zero bits. Individual values are not automatically aligned.

The default type is `unsigned int`, however wide `int` happens to be on the current platform.†

### Endianness

 * `native`

 * `little`

 * `big`

The default endianness is `native`.

**Endianness affects *only* 16-, 32-, and 64-bit types!**†

### Importing Other Files

* `file("filename")`

The file function reads the named file into the output, in-line.

### Compiler State

The compiler state consists of the current type and endianness. It can be saved and restored with the following commands:

<table>
<tr><th>Command</th><th>Description</th></tr>
<tr><td><code>{</code></td><td>Push the compiler state.</td></tr>
<tr><td><code>}</code></td><td>Pop the compiler state.</td></tr>
</table>

These are perhaps best explained by example. Here, `10`, `20`, and `30` will be written as `u32`, while `1` through `6` will all be written as `u8`:

```
u32
10 { u8 1 }
20 { u8 2 3 }
30 { u8 4 5 6 }
```
