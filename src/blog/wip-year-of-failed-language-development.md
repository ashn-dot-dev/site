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
I figured that automatic reference counting (ARC) with all data being assigned
by value provide a mechanism automatic memory management while avoiding the
issue of circular references.
Data organized in this way naturally forms a tree which, if implemented as a
[persistent data structure](https://en.wikipedia.org/wiki/Persistent_data_structure#Trees),
would allow for copy-on-write (COW) to be used as a way to efficiently store
data without paying a huge cost to copy large and/or deeply nested data
structures defined by the user.

This all sounded pretty good on paper, but when it came to producing a working
prototype days turned into weeks turned into months, and after a year of
development, five unsuccessful implementation attempts, and dozens of iterations
on the core data model I just could not find an elegant and efficient way to
implement my vision of the language.
I have obsessed over this problem for over twelve months now, and at the end of
it all I feel like I have nothing to show for my efforts.

This failure isn't the end of the world, and I feel that it certainly wont be
the end of my involvement with programming language design.
It just really sucks to put so much work into a project only to feel like that
energy went to waste.
I am espepecially frusturated because just prior to this project I spent a year
and a half writing a compiler for a C-like language that also ended up being a
flop (or rather just C with a coat of paint).
Two big duds in a row like this have left me with some PL development burnout,
and I think it might be best for me to focus on other projects for a while.
