Sunder is a Sufficiently Stable Programming Language
====================================================

This week I tagged what I consider to be the [first stable
release](https://github.com/ashn-dot-dev/sunder/releases/tag/2022.07.13) of
[Sunder](https://github.com/ashn-dot-dev/sunder), my C-like systems programming
language and compiler for x86-64 Linux. The Sunder project does not use
semantic versioning, and I am not going to make any future compatibility
promises, but I can say that as someone who has spent hundreds of hours
reading, writing, and thinking about Sunder code, this is the first release of
the project where everything *feels* like it is in the right place. The Sunder
[issue tracker](https://github.com/ashn-dot-dev/sunder/issues) has no
outstanding bugs (at the time of writing). The Sunder [standard
library](https://github.com/ashn-dot-dev/sunder/tree/2022.07.13/lib/std)
contains enough tools to write non-trivial software while leaving plenty of
room for future development. And the Sunder compiler seems to be relatively
jank-free[^1].

It is difficult to convey just how incredible of a journey it has been working
on Sunder over this past year and a half. Although I have been in the
programming language space for a number of years, Sunder is the first PL
project of mine that has felt like a true success. Sunder is not just a *good*
programming language, it is my *favorite* programming language. It is the
language that I wish C and C++ could be. It is a language that is an absolute
joy to develop with, and a language that I look forward to using for years to
come.

With this stable-ish language milestone achieved, the focus of Sunder will
shift away from language and standard library development to compiler cleanup
and portability concerns. Currently, Sunder can compile programs for any
architecture and operating system as long as it is x86-64 Linux. Focusing on a
single architecture and operating system up to this point was definitely the
right choice, but it would be nice to see Sunder code running on some of my
other devices. Additionally, targeting different architectures and operating
systems will help uncover potential language-level bugs and weak standard
library abstractions. It would also be nice to explore more sophisticated
compiler backends such as [QBE](https://c9x.me/compile/) or
[LLVM](https://llvm.org/). The current backend strategy of "just output and
assemble un-optimized NASM code lmao" was the right choice for the language up
to this point, but it might be a good time to investigate potential
alternatives. Sunder is also in desparate need of some documentation. Without
any sort of reference guide the only real way to onboard as a Sunder user is to
read through the [example
programs](https://github.com/ashn-dot-dev/sunder/tree/2022.07.13/examples) and
the [standard library
sources](https://github.com/ashn-dot-dev/sunder/tree/2022.07.13/lib/std) which
is... less than ideal.

On a more personal note, I am going to take a bit of a break from significant
work on the Sunder project now that this stable-ish release is finally out.
Side-project burnout is a serious concern of mine, and it is kind of a miracle
that I was able to work as intensely as I did for a year and a half without
suffering severe fatigue. With a project of this scope there is always a
problem to solve, always code that could use refactoring, always an idea for a
new feature, always **something**. I feel like my brain has been *unable* to
stop thinking about this project since development began, and I need to spend
some time away from the project repository if for no other reason than to shut
my brain up for a while. Now is probably a good time to do some development
*with* Sunder rather than *on* Sunder. Project Euler problems, toy
applications, and blog posts featuring example code written in Sunder are all
great ways that I can enjoy using the language in a low-stress environment
without hyper-fixating on compiler internals.

This release is a big deal for me. My work on Sunder is probably the most
effort I have put into a personal project (programming or otherwise) in my
life, and it is pretty surreal seeing everything come together so nicely. I
would be remiss if I didn't take the opportunity to acknowledge my amazing
partner who has put up with way more compiler ramblings than any person should
ever have to endure. I would also like to thank my wonderful dog who has been
an invaluable companion and source of emotional support over the duration of
this project.

If you would like to check out Sunder then you can find the source code and
installation instructions on [GitHub](https://github.com/ashn-dot-dev/sunder)
with a mirror on [SourceHut](https://git.sr.ht/~ashn/sunder). The language is
developed and tested on Debian Stable, but one *should* be able to get the
Sunder toolchain running on any modern x86-64 Linux system with access to a C99
compiler, `ld`, and either `nasm` or `yasm`[^2]. Cheers, and happy Sundering!

## Footnotes
[^1]:
Every compiler (and really every large software project) is going to have
*some* level of jank. There will always be abstractions that could be cleaner,
error messages that could be clearer, and architectural changes that would
allow for simpler code. It is almost impossible to get everything right on the
first try, and it is perfectly acceptable to take on additional unnecessary
complexity (i.e. jank) within a project if you believe the benefits of that
complexity outweigh the drawbacks within the context of the project as a whole.
As a compiler *author* I know that there is plenty of jank to be fixed within
the compiler, but as a *user* I am generally not exposed to the internal
compiler jank. This shielding of jank from the user is what I mean when I say
the compiler seems to be relatively jank-free.

[^2]:
If not then please send me an email or file an issue on GitHub, it would be
much appriciated! 😊
