Sauce of Sunder: Setup and Installation
=======================================

Welcome to the Sauce of Sunder, a series where we dive into the design and implementation of the
[Sunder programming language](https://github.com/ashn-dot-dev/sunder) and associated tooling.

In this post, we are going to walk through in Sunder setup and installation
procedure. By the time we are done, you should have a working Sunder tool chain
installed on the platform of your choice, ready to begin Sunder development.

## A Brief Bit of Background on Sunder's Supported Platforms

Sunder is a systems programming language stared life as a recreational research
project in the early winter of 2021. In its original form, the Sunder compiler
exclusively targeted x86-64 Linux, generating NASM-flavored assembly with its
own custom ABI/calling convention, and interacting with the host system by
making direct system calls to the Linux kernel. Although the Sunder of today is
a lot more portable than the Sunder of the x86-64 Linux days, many of the
design decisions from that period had a lasting impact on design and
implementation of language which we still observe today.

One of those design decisions, relevant to us in this post, is the Unix-centric
focus of the language, standard library, and tooling. You don't have to dig too
deep to see that Sunder expects to operate within Unixy wold-model. In fact,
the tag line for Sunder is literally is "a modest systems programming language
for Unix-like platforms". At the time of writing, Sunder works out of the box
on x86-64 Linux, ARM64 Linux, ARM64 macOS, on the web via WebAssembly through
Emscripten's Unix-like abstraction layer, and could probably be ported to other
common architectures and/or Unix-like platforms without too much hassel.

Crucially however, Sunder is not natively supported on the most popular desktop
operating system, Windows. So for the rest of this post, and the rest of this
series, we are going to assume that you, Dear Reader, will be interacting with
the language and associated tooling on a supported Unix-like platform. For
Linux and macOS users this probably just means "your computer", but for Windows
users this means interacting with Sunder through either the [Windows Subsystem
for Linux](https://learn.microsoft.com/en-us/windows/wsl/) or a virtual
machine.

## Diving Into Dependencies

Sunder is relatively lightweight in the dependencies required for installation
and nominal operation. The Sunder compiler is written in relatively clean C99,
and the compiler generates GNU-flavored C11 as a "portable assembly" which is
then handed off to the C compiler for backend compilation and linking.
