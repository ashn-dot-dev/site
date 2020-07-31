Introducing Ashn's Utility Library (autil)
==========================================

Today I have released the initial version of
[autil](https://git.sr.ht/~ashn/autil), a collection of data structures and
algorithms for rapid application development in C99.
The C standard library lacks many of the tools useful for general purpose
programming, so in practice a lot of C developers cultivate their own set of
tools to supplement libc.
The autil library is my personal set of these tools that I have built up over
multiple years of C development.

The autil library comes as a single header file in the style of the
[stb libs](https://github.com/nothings/stb) popularized by Sean Barrett.
The library contains a general purpose
[exit-on-failure allocator](https://git.sr.ht/~ashn/autil/tree/05e2248e1a82d56d8e8aa15dad73b4253900f014/autil.h#L119),
utility functions for easily
[reading and writing an entire file](https://git.sr.ht/~ashn/autil/tree/05e2248e1a82d56d8e8aa15dad73b4253900f014/autil.h#L138),
basic
[error logging functions](https://git.sr.ht/~ashn/autil/tree/05e2248e1a82d56d8e8aa15dad73b4253900f014/autil.h#L110),
a
[generic resizable array](https://git.sr.ht/~ashn/autil/tree/05e2248e1a82d56d8e8aa15dad73b4253900f014/autil.h#L154)
implementation similar-ish to C++'s `std::vector`, and a
[generic ordered map](https://git.sr.ht/~ashn/autil/tree/05e2248e1a82d56d8e8aa15dad73b4253900f014/autil.h#L215)
implementation similar-ish to C++'s `std::map`.
There is a lot more I want to add to the library, in particular I want to add
string-view and managed string types that properly handle embedded `NUL` bytes.
So look forward to future revisions of this library adding those goodies.

Something I really tried to do with this library was keep the API dead simple.
Most functions exit on allocation failure.
The `vec` and `map` container types are only exposed as opaque pointers to heap
allocated resources created with `*_new` and destroyed with `*_del`, and all
properties on those containers must be queried with explicit function calls
returning rvalues or un-modifiable lvalues.
Functions are documented with short, human-readable comments, and no function
is so long that you would feel overwhelmed reading its definition.
Basically, I tried to [KISS](https://en.wikipedia.org/wiki/KISS_principle) even
if it meant sacrificing some performance and flexibility.

In the future I hope to expand on the autil library with additional data
structures and algorithms.
Additionally I hope to write a blog post on the implementation of the `vec`
and `map` container types.
I think these containers are a lot nicer to work with than their
[glib](https://developer.gnome.org/glib/stable/glib-Arrays.html)
[counterparts](https://developer.gnome.org/glib/stable/glib-Balanced-Binary-Trees.html)
and I used some neat techniques in their implementation that I would like to
share in detail.

Anyway that's all I have for this blog post.
Take a look at the library and let me know your thoughts.
I very much welcome contributor submissions/suggestions, and I would love to
see examples and projects built using the library.
