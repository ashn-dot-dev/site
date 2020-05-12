A Year of Failed Programming Language Development
=================================================

This post is a difficult one for me to write.
For the past year I have spent hundreds of hours working on a programming
language project that, after this post, I will permanently shelve.

The goal of the project was simple: create a lightweight language for
prototyping and embedded scripting that would run without the use of a garbage
collector.
My hope was that the lack of a garbage collector would lead to some interesting
design decisions, even if the expressive power of the resulting language would
be less than what is found in languages such as Lua or Python.
I figured that automatic reference counting (ARC) with value-assignment
semantics would allow for automatic memory management while avoiding the issue
of circular references.
Data organized in this way naturally forms a tree which, if implemented as a
[persistent data structure](https://en.wikipedia.org/wiki/Persistent_data_structure#Trees),
would allow copy-on-write (COW) for efficient passing around of data.

This all sounded pretty good on paper, but when it came to producing a working
prototype days turned into weeks turned into months, and after a year of
development, five unsuccessful implementation attempts, and dozens of iterations
on the core data model I just could not find an elegant and efficient way to
implement my vision of the language.
I have obsessed over this problem for over twelve months, and in the end I feel
like I have nothing to show for it.

This failure isn't the end of the world, and I feel that it certainly wont be
the end of my involvement with programming language design, but it really sucks
to put so much effort into a project and have nothing to show for it.
It is especially painful for me because just prior to this project I spent a
year and a half of effort into writing a compiler for a C-like language that
also ended up being a flop (or rather just a poor mans C with a coat of paint).
After two duds in a row I think I need to take a break from programming language
development.
