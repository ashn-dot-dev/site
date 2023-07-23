A Fourth Cut in Sunder
======================

Last week I read Alex Kladov's [Three Different
Cuts](https://matklad.github.io/2023/07/16/three-different-cuts.html)
discussing the implementation of `cut` in three different languages: Rust, Go,
and Zig. The `cut` function takes a string and a pattern, and splits the string
around the first occurrence of the pattern:

```
cut("life", "if") = ("l", "e")

cut("key=value", "=") = ("key", "value")

cut("hello", "foo") = no result since "hello" does not contain "foo"
```

The article provides a theoretical signature for `cut` if it were to be added
to Zig's standard library:

```zig
pub fn cut(
    s: []const u8,
    sep: []const u8
) ?struct { prefix: []const u8, suffix: []const u8 } {
    ...
}
```

I *really* like this function signature; it is short, easy to read, and uses an
anonymous struct as a named tuple to clearly describe the bundled data returned
from the function. After reading Kladov's article, I knew that I wanted to add
`cut` to Sunder's standard library, and I chose to use the Zig function
signature as the basis for my Sunder implementation.

The Sunder version of the `cut` signature takes the form:

```
func cut(
    str: []byte,
    separator: []byte
) std::optional[[struct { var prefix: []byte; var suffix: []byte; }]] {
    ...
}
```

Sunder supports byte slices via the type `[]byte`[^1], and supports optional
values with the user-defined `std::optional` type. Coincidentally, anonymous
structs were [added to
Sunder](https://github.com/ashn-dot-dev/sunder/commit/203d581bcf19a0e3b026fbff3f0db8d45bbbfd66)
on the same day that Kladov's article was published[^2], so the Sunder
implementation can use the same named tuple type seen in the Zig signature.

The Sunder standard library already supported a `find` function on byte slices,
so `cut` only ended up taking a handful of lines to implement:

```
func cut(
    str: []byte,
    separator: []byte
) std::optional[[struct { var prefix: []byte; var suffix: []byte; }]] {
    alias T = struct { var prefix: []byte; var suffix: []byte; };

    var index = std::str::find(str, separator);
    if index.is_empty() {
        return std::optional[[T]]::EMPTY;
    }

    var index = index.value();
    return std::optional[[T]]::init_value((:T){
        .prefix = str[0:index],
        .suffix = str[index+countof(separator):countof(str)],
    });
}
```

This implementation of `cut` was added to Sunder's standard library as
`std::str::cut` in commit
[4b0296c](https://github.com/ashn-dot-dev/sunder/commit/4b0296cf19d5a0e7485a8723def6e533f2052ec7).
Shortly after the addition, I was able to
[replace](https://github.com/ashn-dot-dev/scratch/commit/d740a4623496e5b0466727b53bae4a861899c4db)
a bunch of `std::str::split` and `std::str::split_with_allocator` calls with
equivalent `std::str::cut` calls across my scratch repo.
[This](https://github.com/ashn-dot-dev/scratch/commit/d740a4623496e5b0466727b53bae4a861899c4db#diff-ebf6af8333d9bbcc5c66635480993b6fdc67babe34fcdef0fac61e0011372bc2L7-L17)
instance was particularly satisfying to replace. At the time when the code was
originally written, I was frustrated at how inelegant the call to
`std::str::split_with_allocator` was when I knew the result of the split
operation fit into fixed size list of two elements. Looking at that code now,
it is obvious that the operation was a tool for `cut`!

## Footnotes
[^1]:
Sunder *does* have a `u8` type, but `u8` and `byte` are distinct. The two types
have identical size and alignment, but arithmetic operators such as addition
and subtraction will produce a compile-time error when used on `byte` values.
The type `[]u8` means "slice of unsigned 8-bit integers" whereas the type
`[]byte` means "slice of bytes" or "byte string".

[^2]:
Anonymous structs and unions were originally added to Sunder in commits
[203d581](https://github.com/ashn-dot-dev/sunder/commit/203d581bcf19a0e3b026fbff3f0db8d45bbbfd66)
and
[d94fc90](https://github.com/ashn-dot-dev/sunder/commit/d94fc9044ca9d48427de5e7aa4fe05744a954abc)
to simplify type definitions, with the inlining of `std::_result_union` into
`std::result` in commit
[8bccfb1](https://github.com/ashn-dot-dev/sunder/commit/8bccfb1e24dfb01b3cd5f07b1634618d638c47e0)
as a motivating example. It was only *after* reading Three Different Cuts that
I realized anonymous structs and unions could also be useful in defining
one-off named tuples for function return types.
