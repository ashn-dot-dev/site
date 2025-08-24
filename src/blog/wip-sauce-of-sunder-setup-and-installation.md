Sauce of Sunder: Setup and Installation
=======================================

Welcome to the Sauce of Sunder, a series where we dive into the design and implementation of the
[Sunder programming language](https://github.com/ashn-dot-dev/sunder) and associated tooling.

In this post, we are going to walk through the Sunder setup and installation
procedure. By the time we are done, you should have a working Sunder toolchain
installed on the platform of your choice, ready to begin Sunder development.

## A Brief Bit of Background on Sunder's Supported Platforms

Sunder is a systems programming language that started life as a recreational
research project in the early winter of 2021. In its original form, the Sunder
compiler exclusively targeted x86-64 Linux, generating NASM-flavored assembly
with its own custom ABI/calling convention, and interacting with the host
system by making direct system calls to the Linux kernel. Although the Sunder
of today is a lot more portable than the Sunder of the x86-64 Linux days, many
of the design decisions from that period had a lasting impact on the design and
implementation of the language which we still observe in the present.

One of those design decisions, relevant to us in this post, is the Unix-centric
focus of the language, standard library, and tooling. You don't have to dig too
deep to see that Sunder expects to operate within a Unixy world-model. In fact,
the tagline for Sunder is literally "a modest systems programming language for
Unix-like platforms". At the time of writing, Sunder works out of the box on
x86-64 Linux, ARM64 Linux, ARM64 macOS, and on the web via WebAssembly through
Emscripten's Unix-like abstraction layer, and could be ported to other common
architectures and/or Unix-like platforms without too much hassle.

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
and nominal operation. The Sunder compiler is written in clean C99, and
generates GNU-flavored C11 as a "portable assembly" which is then handed off to
a C compiler for backend compilation and linking. In addition to a GNU C
compiler, you will need a standard set of POSIX development utilities to build
the Sunder compiler and run the non-Sunder-compiler tools within the Sunder
toolchain. And of course, you will need Git in order to actually clone the
Sunder repository. Setup specifics for a handful of supported platforms are
provided below. Follow along for your supported platform, and then move on to
the next section.

### Debian/Ubuntu

Run the following from the command line:

```sh
sudo apt update && sudo apt install build-essential git
```

### Fedora

Run the following from the command line:

```sh
sudo dnf update && sudo dnf group install "Development Tools"
```

### macOS

Run the following from the command line:

```sh
xcode-select --install
```

## Building and Installing Sunder

Once you have all dependencies installed, the rest of the Sunder setup process
is relatively straightforward. First, clone the Sunder repository using Git:

```sh
git clone https://github.com/ashn-dot-dev/sunder.git
```

Then `cd` into the Sunder repository:

```sh
cd sunder/
```

From within the Sunder repository, build the compiler and install the Sunder
toolchain:

```sh
make install
```

The `install` target will install the Sunder toolchain into the directory
specified by the `SUNDER_HOME` environment variable, which defaults to
`$HOME/.sunder`. If you would like to install the Sunder toolchain into a
directory other than `$HOME/.sunder`, specify a different `SUNDER_HOME` when
running `make install`:

```sh
make install SUNDER_HOME=/opt/sunder # Install to /opt/sunder
```

Then, after running the `install` target, open up a text editor and add the
following blurb to your `.profile` file located at `~/.profile`:

```sh
export SUNDER_HOME="$HOME/.sunder"
if [ -e "$SUNDER_HOME/env" ]; then
    . "$SUNDER_HOME/env"
fi
```

If you used a `SUNDER_HOME` location other than the default `$HOME/.sunder`,
replace `$HOME/.sunder` with your chosen `SUNDER_HOME` location when editing
your `.profile`. For example, if one had installed Sunder with `make install
SUNDER_HOME=/opt/sunder`, then they would add the following to their `.profile`
instead:

```sh
export SUNDER_HOME="/opt/sunder"
if [ -e "$SUNDER_HOME/env" ]; then
    . "$SUNDER_HOME/env"
fi
```

After you have edited and saved your `.profile` you should either restart your
computer so that the changes to your `.profile` are picked up on login, or
source your `.profile` within your current terminal session with:

```sh
. ~/.profile
```

If everything worked correctly, then you should be able to run `sunder-compile
-h` and see the Sunder compiler's help text. All together, the setup and
install process may look something like the following:

```sh
/tmp$ git clone https://github.com/ashn-dot-dev/sunder.git
Cloning into 'sunder'...
remote: Enumerating objects: 10488, done.
remote: Counting objects: 100% (307/307), done.
remote: Compressing objects: 100% (182/182), done.
remote: Total 10488 (delta 173), reused 208 (delta 115), pack-reused 10181 (from 3)
Receiving objects: 100% (10488/10488), 2.76 MiB | 13.03 MiB/s, done.
Resolving deltas: 100% (7042/7042), done.
/tmp$ cd sunder/
/tmp/sunder$ make install
c99 -o sunder-compile.o -c -DSUNDER_DEFAULT_ARCH=$(sh bin/sunder-platform arch)  -DSUNDER_DEFAULT_HOST=$(sh bin/sunder-platform host)  -DSUNDER_DEFAULT_CC=cc -DNDEBUG sunder-compile.c
c99 -o util.o -c -DSUNDER_DEFAULT_ARCH=$(sh bin/sunder-platform arch)  -DSUNDER_DEFAULT_HOST=$(sh bin/sunder-platform host)  -DSUNDER_DEFAULT_CC=cc -DNDEBUG util.c
c99 -o sunder.o -c -DSUNDER_DEFAULT_ARCH=$(sh bin/sunder-platform arch)  -DSUNDER_DEFAULT_HOST=$(sh bin/sunder-platform host)  -DSUNDER_DEFAULT_CC=cc -DNDEBUG sunder.c
c99 -o lex.o -c -DSUNDER_DEFAULT_ARCH=$(sh bin/sunder-platform arch)  -DSUNDER_DEFAULT_HOST=$(sh bin/sunder-platform host)  -DSUNDER_DEFAULT_CC=cc -DNDEBUG lex.c
c99 -o cst.o -c -DSUNDER_DEFAULT_ARCH=$(sh bin/sunder-platform arch)  -DSUNDER_DEFAULT_HOST=$(sh bin/sunder-platform host)  -DSUNDER_DEFAULT_CC=cc -DNDEBUG cst.c
c99 -o parse.o -c -DSUNDER_DEFAULT_ARCH=$(sh bin/sunder-platform arch)  -DSUNDER_DEFAULT_HOST=$(sh bin/sunder-platform host)  -DSUNDER_DEFAULT_CC=cc -DNDEBUG parse.c
c99 -o order.o -c -DSUNDER_DEFAULT_ARCH=$(sh bin/sunder-platform arch)  -DSUNDER_DEFAULT_HOST=$(sh bin/sunder-platform host)  -DSUNDER_DEFAULT_CC=cc -DNDEBUG order.c
c99 -o ast.o -c -DSUNDER_DEFAULT_ARCH=$(sh bin/sunder-platform arch)  -DSUNDER_DEFAULT_HOST=$(sh bin/sunder-platform host)  -DSUNDER_DEFAULT_CC=cc -DNDEBUG ast.c
c99 -o resolve.o -c -DSUNDER_DEFAULT_ARCH=$(sh bin/sunder-platform arch)  -DSUNDER_DEFAULT_HOST=$(sh bin/sunder-platform host)  -DSUNDER_DEFAULT_CC=cc -DNDEBUG resolve.c
c99 -o eval.o -c -DSUNDER_DEFAULT_ARCH=$(sh bin/sunder-platform arch)  -DSUNDER_DEFAULT_HOST=$(sh bin/sunder-platform host)  -DSUNDER_DEFAULT_CC=cc -DNDEBUG eval.c
c99 -o codegen.o -c -DSUNDER_DEFAULT_ARCH=$(sh bin/sunder-platform arch)  -DSUNDER_DEFAULT_HOST=$(sh bin/sunder-platform host)  -DSUNDER_DEFAULT_CC=cc -DNDEBUG codegen.c
c99 -o bin/sunder-compile -DSUNDER_DEFAULT_ARCH=$(sh bin/sunder-platform arch)  -DSUNDER_DEFAULT_HOST=$(sh bin/sunder-platform host)  -DSUNDER_DEFAULT_CC=cc -DNDEBUG sunder-compile.o  util.o  sunder.o  lex.o  cst.o  parse.o  order.o  ast.o  resolve.o  eval.o  codegen.o
mkdir -p "$HOME/.sunder"
cp -r bin "$HOME/.sunder"
cp -r lib "$HOME/.sunder"
cp env "$HOME/.sunder"
/tmp/sunder$ vim ~/.profile # Update my .profile with SUNDER_HOME="$HOME/.sunder"
/tmp/sunder$ . ~/.profile
/tmp/sunder$ sunder-compile -h
Usage: sunder-compile [OPTION...] FILE

Options:
  -c        Compile and assemble, but do not link.
  -d        Do not invoke the C compiler to compile, assemble, or link.
  -e        Display the Sunder environment and exit.
  -g        Generate debug information in output files.
  -k        Keep intermediate files.
  -L DIR    Add DIR to the linker path.
  -l OPT    Pass OPT directly to the linker.
  -o OUT    Write output file to OUT (default a.out).
  -h        Display usage information and exit.
```

## Testing Hello World

It would be a shame to go through all of this setup and installation without
even getting to run a Sunder program, so before we conclude let's write our
first Sunder program, which will print "Hello, world!" to the terminal.

Open up a new file, `hello.sunder`, in your text editor of choice, and fill
that file with the following text:

```sunder
import "std";

func main() void {
    std::print_line(std::out(), "Hello, world!");
}
```

Then, from the command line, execute the following command:

```sh
sunder-run hello.sunder
```

The `sunder-run` tool will automatically compile and execute simple Sunder
programs. In this case, `sunder-run hello.sunder` will compile and execute our
hello world program, which should look like the following:

```sh
$ cat hello.sunder
import "std";

func main() void {
    std::print_line(std::out(), "Hello, world!");
}
$ sunder-run hello.sunder
Hello, world!
```

## Closing Thoughts

Congratulations on installing Sunder and running your first Sunder program! In
future Sauce of Sunder posts we will use this installed Sunder toolchain to
showcase the behavior of the Sunder language, standard library, compiler, and
related tooling. Thank you for following along, and I will see you in the next
installment of Sauce of Sunder.
