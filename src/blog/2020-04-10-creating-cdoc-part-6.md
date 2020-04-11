Creating a C Documentation Generator - Part 6
=============================================

Welcome to last post in the series in which we design and implement `cdoc`
- a source code documentation tool for the C programming language.
In this post we are going to release the 0.1 version of `cdoc`, WOOOO!
Over the past five posts we have gone from an empty C file to a (mostly)
functional documentation generator (prototype).
We will now perform the magical incantations to bump our version from 0.0 to
0.1 and unleash `cdoc` upon the world!


## Bump the Version and Add a Release Tag
Fortunately, the only code change needed for the 0.1 release of `cdoc` is an
update to the `VERSION` macro that we defined in part 1.
Currently the `VERSION` macro is defined as:

```c
#define VERSION "0.0"
```

With a quick change to:

```c
#define VERSION "0.1"
```

`cdoc` now reports that it is version 0.1:

```sh
$ make clean cdoc
rm -f cdoc cdoc.o
c99 -O0 -g -c cdoc.c
c99 -o cdoc cdoc.o -O0 -g
$ ./cdoc --version
0.1
```

The source code with this change can be found
[here](https://git.sr.ht/~ashn/cdoc/tree/02af60dc908e28a3ed70b3ed9f487ba869016a3a).
To make things all official we will add a git tag pointing to this commit:

```sh
$ git tag v0.1
$ git push origin v0.1
```

And with that, version 0.1 of `cdoc` is released.

## Future Work

Release 0.1 marks the end of this series, but not the end of `cdoc` development.
The goal of this series was to build a documentation generator from scratch, and
after 5+ posts we have done just that.
There is a *lot* of work that needs to be done to bring `cdoc` out of an
alpha-quality state, and I hope to accomplish that work over the coming months
and years.
Here is a small sampling of things on the TODO list:

+ Remove all global state from `cdoc.c`
+ Set up unit & integration tests for functions in `cdoc.c`
+ Add doc-comments for all functions & data structures in `cdoc.c`
    + It's kinda funny how this whole series was about writing a documentation
      generator, but I never thought to actually document the documentation
      gerator `¯\_(ツ)_/¯`
+ Add a README and manual
    + I can't tell people to RTFM if I never WTFM
    + I should probably wait until I test `cdoc` on some projects and make
      updates to the documentation language before spending a ton of time on a
      manual
+ Fix source parsing so that it correctly handles comments
+ Add some way to document the members of a `struct` or `union` with `section`s
  inside the data structure definition
    + Right now all members have to be documented in the doc-comment above the
      data structure which leads to poor information locality
+ Clean up the overall code quality of the project

## Wrapping Up

Thank you so much for joining me on this journey.
I hope you enjoyed reading this series of posts as much as I enjoyed writing
them.
If you have any questions, comments, or concerns please feel free to reach out
to me by email.

Cheers and stay hydrated!
