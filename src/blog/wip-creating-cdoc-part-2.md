Creating a C Documentation Generator - Part 2
=============================================

Welcome to part 2 of a series in which we design and implement `cdoc` - a
source-code documentation tool for the C programming language.
In [part 1](/blog/2020-01-21-creating-cdoc-part-1.html) we created the skeleton
for our program and stubbed out the documentation code with a `cat` clone.
In this post we will define the shape of our documentation language and apply
documentation comments to our example file.

## ~~Ripping off~~ Borrowing from Doxygen
Over the past few years of using Doxygen I have settled into a particular
documentation style that I would like to replicate in `cdoc`.

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
I also prefer to write section text indented starting on the line after a tag
because to me:

```c
//! @param p1
//!     Pointer to the first object.
```

is easier to read than:

```c
//! @param p1 Pointer to the first object.
```

This documentation style would work well for `cdoc` because it seems fairly
straightforward to parse[\[2\]](#ft2) and uses a syntax that many developers are
already familiar with.
To make our lives easier during parsing we are also going to say that every
section of a doc must be declared with a tag.
As a convention the first section of a doc should use a tag that describes the C
construct being documented[\[3\]](#ft3) (where applicable).
The `brief` section in our Doxygen example above:

```c
//! Exchange two objects of equal size.      <--- brief section
//! @param whatever                          <--+ section
//! ...bla                                      |
//! ...bla                                   <--+
//! @param something                         <--+ section
//! ...bla                                      |
//! ...bla                                   <--+
void swap(void* p1, void* p2, size_t size);  <--- code
```

would be written in `cdoc` as:

```c
//! @function swap                           <--+ section
//!     Exchange two objects of equal size.  <--+
//! @param whatever                          <--+ section
//! ...bla                                      |
//! ...bla                                   <--+
//! @param something                         <--+ section
//! ...bla                                      |
//! ...bla                                   <--+
void swap(void* p1, void* p2, size_t size);  <--- code
```

All together the `cdoc` documentation for `swap` would look like:

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
In order to parse the `cdoc` language we need to specify rules defining what
counts as valid `cdoc` documentation.
Real computer scientists™ describe these rules using a
[formal grammar](https://en.wikipedia.org/wiki/Formal_grammar),
perhaps in notation such as
[Bacus-Naur form](https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_Form).
If we were producing a formal language specification or writing a non-trivial
parser then coming up with rigorous grammar would certainly be necessary.
But we are in more of an exploratory "hack it together and see what works"
phase, so I would rather describe our language in simple English and build
parsing code based on that.

For now we will loosely define the `cdoc` documentation language as such:

+ A **`doc`** consists of lines beginning with the characters "`//!`", divided
into one or more `section`s.
+ A **`section`** consists of a line in either of the following forms:
```c
//! @TAG
```
```c
//! @TAG NAME
```
followed by zero or more lines of HTML text.

Pretty straightforward right?

## Revamping the Example File
Now that we have a rough idea of what `cdoc` documentation will look like we
should update our `example.c` file with doc-comments.
In part 1 we threw together our example file at the end of the blog post without
a whole lot of thought.
In retrospect I realize that the file is missing a lot of C constructs
(macros, variables, typedefs, etc.), so while we are adding comments we might
as well just overhaul the entire thing.
With a little bit of work revamped file (with added doc comments) has a lot more
going on and should be a good test file to write a parser against:

```c
//! @file example.c
//!     This is a C source file used to test cdoc.
//! @license 0BSD

#include <stddef.h> // size_t
#include <stdint.h> // [u]intN_t

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

//! @struct string
//!     POD type representing an array-of-char with a known size.
struct string
{
    char* data;
    size_t size;
};

//! @union foobar
//!     A data packet sent over the wire from FooBar Inc.
union foobar
{
    uint32_t foo;
    float bar;
};

//! @enum color
//! @todo
//!     Add more colors.
typedef enum
{
    RED = 0,
    BLUE = 1,
    GREEN = 2
} color;

//! @typedef colour
//!     Convenient typedef for non-American software engineers.
typedef color colour;

//! @function get_color
//! Cdoc should be lenient with whitespace.
//!     Any amount of indenting should be allowed.
//!	Even with tabs.
//! @return
//!     My favorite color.
color get_color(void)
{
    return RED;
}

//! @macro M_PER_KM
#define M_PER_KM 1000

//! @macro KM
//!     Convert meters into kilometers.
#define KM(meters) (meters * M_PER_KM)

//! @macro NUM_FOOBAR
//!     256 is a computer-ish number, right?
#define NUM_FOOBAR 256
//! @variable foobars
union foobar foobars[NUM_FOOBAR];
```

## Putting it all Together
We now have an informal grammar describing our documentation language and a
relatively solid source file we can use as a reference/test-file while
constructing the `cdoc` parser.
In the next post we attempt to get a minimum-viable-product of `cdoc` completed
by transforming `cdoc` documentation into HTML!
I hope to see you then!

The source code for this blog post can be found
[here(TODO-link)](TODO).


## Footnotes
<div id="ft1">\[1\]:
Doxygen uses the term [command](http://www.doxygen.nl/manual/commands.html) for
`@foo` or `\foo`, but I have always heard these referred to as tags.
I am going to use "tag" for this series because I think it is a much clearer
name.
</div>

<div id="ft2">\[2\]:
Spoiler - I have actually already written the parser for `cdoc`, so my intuition
on what should and should not be in the documentation language is a more a
product of trial and error over many weekends rather than any sort of informed
parsing wisdom.
</div>

<div id="ft3">\[3\]:
Although most of `cdoc`'s documentation language is "borrowed" from Doxygen the
convention to use the first tag as a description of the construct being
documented is being lifted from
[NaturalDocs](https://www.naturaldocs.org/getting_started/documenting_your_code/#the_basics),
another high quality documentation generator.
If the Doxygen/Javadoc/Cdoc documentation style is not your cup of tea then I
would give NaturalDocs a look.
</div>
