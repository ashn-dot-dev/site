A Year of Failed Programming Language Development
=================================================

This post is a difficult one for me to write.
For the past year I have spent hundreds of hours working on a programming
language project that, as of this post, I will permanently shelve.

The goal of the project was simple: create a lightweight language for
prototyping and embedded scripting that could run without a garbage collector.
My hope was that the lack of garbage collection would lead to some interesting
design decisions, even if the expressive power of the resulting design would be
lacking in comparison to a language such as Lua or Python.
I figured that automatic reference counting with all data being assigned by
value would provide a mechanism for automatic memory management while avoiding
the issue of circular references.
Data organized in this way naturally forms a tree which, if implemented as a
[persistent data structure](https://en.wikipedia.org/wiki/Persistent_data_structure#Trees),
would allow for copy-on-write to be used as a way avoid cloning an entire
sub-tree with every assignment.

This all sounded pretty good on paper, but when it came to producing a working
prototype days turned into weeks turned into months, and after a year of
development, five unsuccessful implementation attempts, and dozens of iterations
on the core data model I just could not find an elegant and efficient way to
implement my vision of the language.
Just to clarify - it is possible to create a dynamic language without garbage
collection: [Bone-Lisp](https://github.com/wolfgangj/bone-lisp) and
[Dyon](https://github.com/PistonDevelopers/dyon) are two neat languages that
run without the use of a garbage collector.
But the trade-offs that these languages make, the very same trade-offs that I
had to consider in my own language, always left me unsatisfied.
Perhaps there is an elegant, simple, and performant way to deliver a zenful
Pythonic experience in a language without garbage collection, but if so then I
do not think I will be the one to provide that experience.

I have obsessed over this problem for more than twelve months and in the end
all I have to show for my efforts is thousands upon thousands of lines of
abandoned code.
This failure isn't the end of the world, and I feel that it certainly wont be
the end of my involvement with programming language design.
It just really sucks to put so much work into a project only to feel like that
energy went to waste.
