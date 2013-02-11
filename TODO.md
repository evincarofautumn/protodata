# Macros and Expressions

Macro definitions such as:

```
c_string(str){str {u8 0}}
pascal_string(type, str){{type length(str)} str}
```

Expressions with `+`, `-`, `*`, `/`, and `%` operators, as
well as the following sorts of functions:

 * `here()`

   The current output position.

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
