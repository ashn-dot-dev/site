Creating a C Documentation Generator - Part 2
=============================================

Welcome to part 2 of a series in which we design and implement `cdoc` - a
source-code documentation tool for the C programming language.
In [part 1](/blog/2020-01-21-creating-cdoc-part-1.html) we created the skeleton
for our program and stubbed out the documentation code with a pseudo-`cat`
implementation.
In this post we will define the shape of our documentation language and prepare
ourselves for writing `cdoc`'s parser.

## ~~Ripping off~~ Borrowing from Doxygen
Over the past several years of using Doxygen I have settled into a particular
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
//!     E.g `swap(&foo, &foo, sizeof(foo))` is valid.
void swap(void* p1, void* p2, size_t size);
```

using `//!` [comment blocks](http://www.doxygen.nl/manual/docblocks.html)
and Javadoc style tags[\[1\]](#ft1).
I also prefer to write section text indented started on the line after a tag
because I have found that:

```c
//! @param p1
//!     Pointer to the first object.
```

is easier for me to read than:

```c
//! @param p1 Pointer to the first object.
```

This documentation style would work well for `cdoc` because it uses a syntax
that many developers are already familiar with and seems fairly straightforward
to parse[\[2\]](#ft2).
To make our lives easier we are also going to say that every section of a doc
must be declared with a tag, and that as a convention the first section of a
doc should be written with a tag that describes the C construct being documented
(where applicable) in more of a
[NaturalDocs](https://www.naturaldocs.org/getting_started/documenting_your_code/#the_basics)
style.
In our swap example the initial `brief` section:

```c
//! Exchange two objects of equal size.      <--- brief section
//! @param whatever                          <--+
//! ...bla                                      + section
//! ...bla                                   <--+
//! @param something                         <--+
//! ...bla                                      + section
//! ...bla                                   <--+
void swap(void* p1, void* p2, size_t size);  <--- Code
```

would be written as:

```c
//! @function swap                           <--+ section
//!     Exchange two objects of equal size.  <--+
//! @param whatever                          <--+
//! ...bla                                      + section
//! ...bla                                   <--+
//! @param something                         <--+
//! ...bla                                      + section
//! ...bla                                   <--+
void swap(void* p1, void* p2, size_t size);
```

All together the `cdoc` documentation for the `swap` would look like:

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
//!     Objects p1 and p2 may point to overlapping memory.
//!     E.g swap(&foo, &foo, sizeof(foo)) is valid.
void swap(void* p1, void* p2, size_t size);
```

## Informal Grammar
In order to parse our documentation language we are going to need to specify
a set of rules to describe what is and is not valid `cdoc` input to `cdoc`.
Real computer scientists™ describe these rules using a
[formal grammar](https://en.wikipedia.org/wiki/Formal_grammar),
perhaps in notation such as
[Bacus-Naur form](https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_Form).
If we were producing a formal language specification or writing a non-trivial
parser then coming up with rigorous grammar would certainly be necessary,
But we are in more of an exploratory "hack it together and see what works"
phase, so I would rather describe our language in simple English and build our
parsing code based on that.

A `comment-line` is a line of source code beginning with the characters `//!`.
A `section` is sequence of one or more consecutive `comment-line`s.
The first line of a `section` must the form `//! @TAG` or the form
`//! @TAG NAME`, i.e. `NAME` is optional.
A `doc` is a sequence of one or more consecutive `section`s.
And for good measure we will say that whitespace is lenient: all lines of a
`doc` may have leading or trailing horizontal whitespace.

## Revamping the Example File


## Putting it all Together
The source code for this blog post can be found
[here(TODO-link)](TODO).


## Footnotes
<div id="ft1">\[1\]:
Doxygen uses the term [command](http://www.doxygen.nl/manual/commands.html) for
`@foo` or `\foo`, but I have always heard these referred to by the name "tag".
I am going to use the term "tag" for this series because I think it is a much
clearer name.
</div>

<div id="ft2">\[2\]:
Spoiler - I have actually already written the parser for `cdoc`, so my intuition
on what should and should not be in the documentation language is a more a
product of trial and error over many weekends rather than any sort of informed
parsing wisdom.
</div>
