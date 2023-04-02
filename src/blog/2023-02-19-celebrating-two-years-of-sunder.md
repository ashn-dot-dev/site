Celebrating Two Years of Sunder
==============================

Two years ago today I began working on the project that would eventually become
[Sunder](https://github.com/ashn-dot-dev/sunder), a C-like systems programming
language and compiler for x86-64 Linux. A lot has happened in the twelve months
since
[Sunder's first birthday](/blog/2022-02-19-celebrating-one-year-of-sunder.html).
If year one was about laying a solid foundation for the Sunder project, then
year two was about getting Sunder to be usable as a general purpose programming
language. Over the past year Sunder has seen use in recreational programming
problems[^1], a barebones x86-64 operating system kernel[^2], and a proof of
concept interpreter[^3] that have all helped to guide the language and standard
library to its current form. Sunder reached a point this past summer at which
the language could be considered
[sufficiently stable](/blog/2022-07-14-sunder-is-a-sufficiently-stable-programming-language.html)[^4],
and although there will always be a seemingly endless supply of work to be done
improving the language and standard library, I am absolutely thrilled with
where the project is after two years of development. Happy second birthday
Sunder!

## Footnotes
[^1]:
Including problems from
[Advent of Code 2015](https://github.com/ashn-dot-dev/scratch/tree/main/advent-of-code-2015),
[Advent of Code 2022](https://github.com/ashn-dot-dev/scratch/tree/main/advent-of-code-2022),
and
[Project Euler](https://github.com/ashn-dot-dev/scratch/tree/main/project-euler).

[^2]:
[SundOS](https://github.com/ashn-dot-dev/sundos), a barebones "hello world"
kernel using the Limine bootloader. This project is really cool because the
kernel is written entirely in Sunder!

[^3]:
Currently existing as the
[calculator language](https://github.com/ashn-dot-dev/sunder/blob/2023.02.19/examples/big-integer-calculator.sunder)
example within the Sunder repository.

[^4]:
Sunder has received a few major updates since the "Sunder is a Sufficiently
Stable Programming Language" blog post, including a keyword change (`const` to
`let`), as well as a standard library allocator rework. However, the core
language and standard library are still very much the same.

[^5]:
Some of which are currently in the works!
