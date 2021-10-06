I Wrote a Compiler for My Own Programming Language
==================================================

This past Friday I officially tagged the [first
release](https://github.com/ash-nolan/sunder/releases/tag/2021.10.01) of
[Sunder](https://github.com/ash-nolan/sunder), a C-like systems programming
language and compiler for x86-64 Linux. Here is "Hello World" in Sunder:

```txt
import "std/io.sunder";

func main() void {
    std::println("Hello, world!");
}
```

I have been working on Sunder for a little over half a year now[^1]. Although
the project is still in its infancy, the language and compiler already support
an impressive number of features including, but not limited to:

Comments:
```txt
# this is a comment in Sunder
```
Variables:
```txt
var foo: byte = 0xFF;
```
Compile-time constants stored in read-only memory:
```txt
const bar: byte = 0xFF;
```
Functions with 0 to N parameters and a single return type/value:
```txt
func max(a: ssize, b: ssize) ssize {
    if a > b {
        return a;
    }
    return b;
}
```
Data types including:

+ `void` (zero-sized type)
+ `bool` (boolean type)
+ `byte` (8-bit data that does not support integer arithmetic)
+ `u8`, `s8`, `u16`, `s16`, `u32`, `s32`, `u64`, `s64` (explicitly sized
  unsigned and signed integers)
+ `usize` and `ssize` (architecture-dependent unsigned and signed integers
  capable of losslessly storing a pointer)
+ Functions:
  ```txt
  const fn: func(ssize, ssize) ssize = max;
  ```
+ Pointers:
  ```txt
  const pointer: *byte = &foo;
  ```
+ Arrays:
  ```txt
  const array: [3]byte = (:[3]byte)[3, 5, 7];
  ```
+ Slices (pointer plus length, sometimes called a "fat pointer"):
  ```txt
  const slice: []byte = array[0:3];
  ```

Expressions including:

+ Boolean literals:
  ```txt
  true;
  false;
  ```
+ Integer literals in decimal, binary, octal,  and hexadecimal notation:
  ```txt
  123;    # decimal
  0b1010; # binary
  0o0755; # octal
  0xBEEF; # hexadecimal
  ```
+ String literals with type slice-of-byte):
  ```txt
  "this is a string (a.k.a bytes) literal";
  ```
+ Array literals:
  ```txt
  (:[3]byte)[3, 5, 7];    # 3, 5, 7
  (:[5]byte)[3, 5, 7...]; # 3, 5, 7, 7, 7
  ```
+ Slice literals (explicitly constructed from a pointer and length):
  ```txt
  (:[]byte){&array[0], 2}; # 3, 5
  ```
+ Casting between `bool`, `byte`, integer, and pointer values:
  ```txt
  const baz: usize = (:usize)bar;
  ```
+ Raw system calls:
  ```txt
  syscall(60s, 0s); # exit(0) on x86-64 Linux
  ```
+ Array and slice indexing:
  ```txt
  array[1]; # 5
  slice[1]; # 5
  ```
+ Array and slice slicing:
  ```txt
  array[0:2]; # elements of `array` within the half-open range [0, 2)
  slice[0:2]; # elements of `slice` within the half-open range [0, 2)
  ```
+ The `sizeof` operator:
  ```txt
  sizeof(:s32);
  ```
+ Short circuiting boolean operators `not`, `or`, and... uhh `and`:
  ```txt
  true and false or not true;
  ```
+ Logical operators `==`, `!=`, `<`, `<=`, `>`, and `>=`:
  ```txt
  a == b;
  a != b;
  a < b;
  # etc.
  ```
+ Bitwise operators `|`, `^`, and `&`:
  ```txt
  a | b;
  a ^ b;
  a & b;
  ```
+ Arithmetic operators `+` (unary and binary), `-` (unary and binary), `*`,
  and `/`:
  ```txt
  +a;
  -a;
  a + b;
  a - b;
  a * b;
  a / b;
  ```
+ Pointer operators `&` (unary addressof) and `*` (unary dereference):
  ```txt
  var ptr_to_foo: *byte = &foo;
  var val_at_ptr: byte = *ptr_to_foo;
  ```
+ The `countof` operator which evaluates to the number of elements in an array
  or slice:
  ```txt
  countof(array);      # 3
  countof(array[1:3]); # 2
  ```

If statements (using the keywords `if`, `elif`, and `else`):
```txt
if condition {
  # body
}

if condition0 {
  # body
}
elif condition1 { # there may be 0 -> N elif components
  # body
}
else { # there may be at most one else component
  # body
}
```

Looping constructs including:

+ For-expression statements similar to `while` in C:
  ```txt
  for expression {
      # body
  }
  ```
+ For-range statements similar to the idiomatic
  `for (size_t i = begin; i < end; ++i) { /* body */ }` in C:
  ```txt
  for i in begin:end {
      # body
  }
  ```
+ In-loop `break` and `continue` statements:
  ```txt
  var i: usize = 10;
  for true {
      i = i + 1;
      if i == 15 {
          break;
      }
      continue;
      std::println("UNREACHABLE!");
  }
  ```

Dump statements which write the raw bytes of an object to standard error
(really useful for debugging):
```txt
dump (:[3]u16)[0xAB, 0xBC, 0xDE]; # AB 00 BC 00 DE 00
```

Return statements:
```txt
return 123;
```

Assignment statements (assignment is a statement and not an expression):
```txt
foo = 0xAA;
```

Namespaces:
```txt
# lib/std/io.sunder
namespace std;
```

Module imports and multi-file compilation:
```txt
import "std/io.sunder";
```

The `typeof` operator which evaluates to the type of an expression:
```txt
var m: ssize = 123;
var n: typeof(m) = m + 1;
```

Automatic index out-of-bounds, integer overflow, integer underflow, and
divide-by-zero checking:
```txt
# When this expression is executed the program will print the following error
# message and exit:
#
# fatal: arithmetic operation produces out-of-range result
0xFFFFu16 + 1u16;
```

Constant expression evaluation:
```txt
const x: u16 = 123 + 456 * 2;
const y: [1 + 2]u16 = (:[3]u16)[0...];
```

Order-independent top-level declarations:
```txt
const c: ssize = b + 1s;
const a: ssize = 1s;
const b: ssize = a + 1s;
```
And more!

The Sunder compiler and standard library try to do as much from scratch as
reasonably possible. The compiler is written in strict C99 and requires only
`nasm` and `ld` for assembling and linking. Sunder executables are statically
linked without any dependency on libc:

```sh
~$ cd ~/sources/sunder/
~/sources/sunder$ sunder-compile -o hello examples/hello.sunder
~/sources/sunder$ ./hello
Hello, world!
~/sources/sunder$ ldd ./hello
	not a dynamic executable
```

I should be clear that Sunder is **not** a wannabe C replacement; I am working
on this project purely as an academic exercise. Sunder is still very much a
"toy" programming language and I don't have the time, energy, or desire to
partake in the [race to replace
C](https://media.handmade-seattle.com/the-race-to-replace-c-and-cpp/). However,
the project is built on a solid foundation and I am looking forward to improving
on the language, compiler, and standard library over the coming weeks, months,
and (maybe) years.

There is no shortage of work to be done on the Sunder project. The language has
no concept of user defined `struct` types, nor does it have a concept of
generics, both of which are required to develop the abstractions and interfaces
I would like to have within Sunder's standard library. There is also a fair bit
of cleanup needed in the compiler. Although I am extraordinarily pleased with
the compiler's overall architecture, it is almost impossible to get everything
right on the first try, and the project has accrued enough technical debt to
warrant a few refactoring passes before I go around adding any new features.
That being said, this project has consumed my nights and weekends for the past
seven or so months, and having reached the major milestone of a first release, I
think I am going to take a bit of a break to focus more on my family and
personal (non-side-project) life for a little while. I will still be working on
Sunder, but in the near future my effort on this project is going to be mostly
focused on piecemeal refactoring of the compiler without any drastic changes to
the language or standard library.

Anyway that's all I have for this blog post. For those interested, the main
Sunder repository and issue tracker can be found on
[GitHub](https://github.com/ash-nolan/sunder) and a mirror can be found on
[SourceHut](https://git.sr.ht/~ashn/sunder). I am *extremely* happy with how
this project has turned out so far and I am super proud of how much I have grown
as a language and compiler author. Cheers!

## Footnotes
[^1]:
A backup copy of the project repository shows that the initial commit occurred
on 2021-02-19.
