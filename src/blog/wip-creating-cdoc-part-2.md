Creating a C Documentation Generator - Part 2
=============================================

Welcome to part 2 of a series in which we design and implement `cdoc` - a
source-code documentation tool for the C programming language.
In [part 1](/blog/2020-01-21-creating-cdoc-part-1.html) we created the skeleton
for our program and stubbed out the documentation code with a pseudo-`cat`
implementation.
In this post we will define the shape of the minimal documentation that will be
processed by our application.

## ~~Ripping off~~ Borrowing from Doxygen
Over the past several years off using Doxygen I have settled into a particular
style of documentation that I would like to replicate in `cdoc`.

Take for example an idiomatic swap function:

```c
void swap(void* p1, void* p2, size_t size);
```

I would write a Doxygen doc for `swap` as:

```c
//! Exchange two objects of equal size.
//! @param p1
//!     Pointer to the first object.
//! @param p2
//!     Pointer to the second object.
//! @param size
//!     The sizeof both objects.
//! @note
//!     Objects @p1 and @p2 may point to overlapping memory.
void swap(void* p1, void* p2, size_t size);
```

using `//!` [comment blocks](http://www.doxygen.nl/manual/docblocks.html)
and Javadoc style tags[\[1\]](#ft1).
Also note that I prefer to indent the text after a tag on a separate line.
This documentation style would work well for `cdoc` because takes minimal effort
to parse correctly and uses the Javadoc syntax that many developers already have
a familiarity with.

One thing I do want to change is the very first brief section of a doc.
Documentation generators such as Doxygen and Javadoc parse the source language
in addition to documentation comments; Doxygen knows that `swap` is a function
because it has a fairly sophisticated C (really C++) parser providing that
information.
We **really** do not want to go down the rabbit hole of implementing a C parser,
so we will require the user tell us what C construct the doc refers to:

```c
//! @function swap
//!     Exchange two objects of equal size.
//! @param p1
//!     Pointer to the first object.
//! @param p2
//!     Pointer to the second object.
//! @param size
//!     The sizeof both objects.
//! @note
//!     Objects @p1 and @p2 may point to overlapping memory.
void swap(void* p1, void* p2, size_t size);
```

## Informal Grammar
Our documentation language is... well... a language, and every language has a
grammar - the set of rules describing what is and is not a valid input accepted
by the language.
Real computer scientists™ describe these rules using a
[formal grammar](https://en.wikipedia.org/wiki/Formal_grammar),
perhaps in notation such as
[Bacus-Naur form](https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_Form).

If we were producing a formal language specification or writing a non-trivial
parser then coming up with rigorous grammar would certainly be necessary.
But we are in more of an exploratory "hack it together and see what works"
phase, so I would rather describe our language in simple English and build our
parsing code based on that.

TODO: Figure out good way to describe in plain English.

+ `doc`
+ `section`
+ `line`

Each line of a `doc` / `section` must begin with `//!`.

A `section`'s first line must take the form `//! @TAG` or the
form `//! @TAG NAME`, i.e. `NAME` is optional.

A `section`'s remaining lines are treated as HTML.

And for good measure we will say that whitespace is lenient: the lines of a
doc comment may have leading or trailing horizontal whitespace and amount
of horizontal whitespace is treated as one whitespace character.

## Putting it all Together
TODO: Show generated HTML.

The source code for this blog post can be found
[here(TODO-link)](TODO).


## Footnotes
<div id="ft1">\[1\]:
Doxygen uses the term [command](http://www.doxygen.nl/manual/commands.html) for
`@foo` or `\foo`, but I have always heard these referred to by the name "tag".
I am going to use the term "tag" for this series because I think it is a much
clearer name.
</div>
