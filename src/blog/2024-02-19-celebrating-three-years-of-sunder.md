Celebrating Three Years of Sunder
=================================

Three years ago today I began working on the project that would eventually
become [Sunder](https://github.com/ashn-dot-dev/sunder). Today we celebrate
what will hopefully be the last (major)
[release](https://github.com/ashn-dot-dev/sunder/releases/tag/2024.02.19) for
the Project! That's right, after three years full of blood, sweat, and defining
language semantics, Sunder is now getting a send-off into the world of Systems
Programming.

With this latest release, the plan is to freeze the language and standard
library in its current state -- warts and all. Sunder has been stable-ish for
the past year or so, but occasional breaking changes have made it difficult for
me to invest time into documentation knowing that anything I write might not be
accurate in the future. I have also been experimenting with
[Sunder](https://github.com/ashn-dot-dev/raylib-sunder)
[language](https://github.com/ashn-dot-dev/nbnet-sunder)
[bindings](https://github.com/ashn-dot-dev/smolui)
to C libraries, a
[Sunder-native library](https://github.com/ashn-dot-dev/bubby),
and even a
[networked Settlers of Catan clone](https://github.com/ashn-dot-dev/natac).
Moving forward, I would like to shift my focus more to these projects without
having to worry about the language and standard library shifting beneath my
feet.

There is still a good bit of work that can be done to improve the compiler, and
there are definitely a few places in the standard library that could use some
code cleanup. Over the coming year(s) I will probably still commit to the
Sunder repository to fix bugs, clean up backend code generation[^1], and add
additional operating systems to the list of supported platforms[^2]. But for
all practical purposes, the Sunder project is complete. Sunder is everything
that I ever wanted it to be and more. I think that after three years of hard
work, it is time to take a break and enjoy what has become my absolute favorite
programming language.

Happy third birthday Sunder!

## Footnotes
[^1]:
Which is currently fine for the set of Unix-like platforms supported, but
relies on having a GNU C compiler such as GCC or Clang available. I am hoping
that the additions coming in the C23 standard, coupled with a switch from GNU C
statement expressions to some form of destination-driven code generation in
emitted C code, will be enough for the complier to emit standards-compliant C
code during code generation.


[^2]:
I would really like to write a blog post about adding FreeBSD support to Sunder
some time this year!
