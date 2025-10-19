The Mellifera Master Plan to Maximize Portability and Performance
=================================================================

I have been developing [Mellifera](https://github.com/ashn-dot-dev/mellifera)
for about a month now, during which time the language has shaped up into
something that I find quite pleasant to work with. My brain naturally thinks
about code and data in terms of value semantics, so it is really lovely to have
a scripting language where value semantics are at the heart of the programming
model.

```mellifera
let a = ["foo", "bar", "baz"];
let b = a; # b is a separate copy of a (this makes my brain happy)
a.push("lasagne");

println($"a is {a}");
println($"b is {b}");
```

```sh
$ mf value-semantics.mf
a is ["foo", "bar", "baz", "lasagne"]
b is ["foo", "bar", "baz"]
```

The Mellifera value system, core types, and a decent number of builtin
functions have already been implemented in the reference interpreter and the
language is now at a point where one could reasonably use it for the same
flavor of ad-hoc command line tools that would typically be handled by
something like Perl or POSIX Shell, albeit with more verbosity and less
out-of-the-box functionality.

```sh
$ # some arbitrary selection of English words
$ cat /usr/share/dict/american-english | head -n 1000 | tail -n 10
Appleseed
Appleseed's
Appleton
Appleton's
Appomattox
Appomattox's
Apr
April
April's
Aprils

$ # that same selection with words of the form "${WORD}'s" removed using Unix tools
$ cat /usr/share/dict/american-english | head -n 1000 | tail -n 10 | sed "s/'s//g" | uniq
Appleseed
Appleton
Appomattox
Apr
April
Aprils

$ # that same selection with words of the form "${WORD}'s" removed using Mellifera
$ cat /usr/share/dict/american-english | head -n 1000 | tail -n 10 | \
  mf --command 'for word in input().trim().split("\n").iterator().filter(function(word) { return word !~ r"\x27s"; }) { println(word); }'
Appleseed
Appleton
Appomattox
Apr
April
Aprils
```

So this is cute right. Like, if this was an undergraduate capstone project or
one of my usual programming language research projects, then now would be a
good time to stop and say "here is this cool thing I made, and this is what I
learned from it". But I actually want to *use* this language outside of an
academic or one-off project context, so instead of wrapping things up I find
myself asking the question "what are the next steps needed in order to take
this language from a research project and make it into something actually
usable?", which believe it or not is something I have never really done before.

At the time of writing, Mellifera, specifically Mellifera as implemented by the
current reference interpreter written in Python, has a few major hurdles to
overcome, all of which fall into two broad categories of **portability** and
**performance**. I've used the phrase "reference interpreter" a couple of times
now without explaining what I mean by that terminology, so before we dive into
these portability and performance challenges, I want to take a moment to talk
about this Mellifera refrence interpreter, and about how Mellifera is currently
developed and deployed.

## The Current Mellifera Reference Interpreter

Looking at the [Mellifera
repository](https://github.com/ashn-dot-dev/mellifera/tree/df1d59a822052c3f01b641576b04a53d9f46a6d9)
on the day of this blog post publication, we see a relatively simply Python
project structure with corresponding Python-flavored [development
instructions](https://github.com/ashn-dot-dev/mellifera/blob/df1d59a822052c3f01b641576b04a53d9f46a6d9/README.md#development).
In this repository, the Mellifera interpreter is implemented within a single
Python script,
[`mf.py`](https://github.com/ashn-dot-dev/mellifera/blob/df1d59a822052c3f01b641576b04a53d9f46a6d9/mf.py).
This Python script is the thing I have been referring to at the "reference
interpreter", and is, at least at the time of writing, the authoritiative
definition of the Mellifera language. That is to say, the behavior of a
Mellifera program **is** whatever the behavior of this Python script does when
executing that program.

When standalone Mellifera tooling is installed via `make install`, the `mf.py`
script is compiled into a native executable via [Nuitka](https://nuitka.net/),
but under the hood everything is still just Python. Look you can even do a
horrifying trick at runtime by executing new Python code from within the
Mellifera runtime itself.

```mellifera
println("This is plain ol' Mellifera code.");
println("Mellifera normally does not have a way to execute web requests!");
println("================================");
py::exec(```
import urllib.request
import json
request = urllib.request.Request(
    "https://icanhazdadjoke.com/",
    headers={'Accept': "text/plain"}
)
response = urllib.request.urlopen(request)
print(response.read().decode("UTF-8"))
```);
```

```sh
$ mf py-exec.mf
This is plain ol' Mellifera code.
Mellifera normally does not have a way to execute web requests!
================================
What did one plate say to the other plate? Dinner is on me!
```

Having this singular language implementation written in Python means that
Mellifera as a whole tends to share many of the same benefits and drawbacks
seen in other Python-based projects. Iteration times are quick, the package
ecosystem is excellent, and tooling support (at least on Unix-like systems)
tends to be pretty great. However, runtime performance of pure Python code
leaves a lot to be desired, packaging and deployment is a pain, and code
cleanliness requires a continuous active mental effort to upkeep due to the
loosey-goosey nature of the Python language. So in the next sections when I
talk about the challenges that the Mellifera project needs to overcome, keep in
mind that many of these challenges are directly tied to the drawbacks of
implementing a language like Mellifera in Python.

## Portability and Performance Challenges in Mellifera

Mellifera as it exists today is a *decent* programming language that is held
back by what I would frankly consider to be show-stopping usability problems
related to the portability and performance of the language and interpreter.
Just to be absolutely clear, I do not think I could recommend Mellifera for use
as a general purpose scripting language until all of these issues are
addressed, so half-assed solutions are not going to cut it here. From my
perspective, the primary challenges facing Mellifera are:

+ Unacceptably slow interpreter performance
+ A Standard library that is too minimalist for an ad-hoc scripting language
+ Poor deployment and embedding story

### Unacceptably Slow Interpreter Performance

I feel like scripting languages get a little bit of a pass when it comes to
runtime performance. Like, if someone is writing a one-off throwaway script in
Python, then chances are they probably do not care that much if it executes
100x slower than the equivalent code written in a language like C++ or Rust. Of
course there will always be outliers like LuaJIT and PyPy which execute their
respective scripting languages blazingly fast™, but in general it seems like
there is this tradeoff between convenience and performance where scripting
languages tend to favor convenience. Of course there is going to be some point
at which no amount of convenience will make up for a program just taking too
damn long to run, and for Mellifera that point is "all of the time". Just take
a look at the execution time of "hello world" in Perl, Python, and Mellifera.

```sh
$ time perl -e 'print "Hello, World!\n";'
Hello, World!

real    0m0.003s
user    0m0.000s
sys     0m0.001s
$ time python3 -c 'print("hello world");'
hello world

real    0m0.009s
user    0m0.006s
sys     0m0.003s
$ time mf -c 'println("hello world");'
hello world

real    0m0.080s
user    0m0.068s
sys     0m0.008s
```

Perl and Python execute so quickly that I am not even able to perceive a period
in which their interpreters are actively running. Compare that Mellifera where
it takes almost a tenth of a second to print "hello world" on a fairly powerful
machine. The Mellifera startup and execution time is a huge red flag! And it
gets even worse with larger programs containing more compute-heavy operations.
Consider the example word counting program featured in the Mellifera project
README. Executing this program on a realtively small input is over 250x slower
than running a similar command builtin from programs strung together in Shell.

```sh
$ curl -s https://www.gutenberg.org/files/71/71-0.txt >gutenberg-71.txt
$ time (cat gutenberg-71.txt | tr '[:upper:]' '[:lower:]' | tr -s '[:space:]' '\n' | sed 's/[^[:alnum:]_]//g' | grep -v '^$' | sort | uniq -c | sort -rn | head -5 | awk '{print $2, $1}')
the 692
to 440
and 418
of 391
a 293

real    0m0.008s
user    0m0.010s
sys     0m0.006
$ time (cat gutenberg-71.txt | mf examples/word-count.mf --top 5)
the 692
to 440
and 418
of 391
a 293

real    0m2.057s
user    0m2.015s
sys     0m0.040s
```

So why is Mellifera so slow? Like seriously, how could the interpreter
performance be so bad that it takes two whole seconds to count the number words
in a text file that is...

```sh
$ stat -c '%n %s' gutenberg-71.txt
gutenberg-71.txt 7224
```

...**ONLY 71K IN SIZE!?**

Well, the reference interpreter is basically receiving two gut punches to
performance here. Firstly, the interpreter is written in Python, which is
itself an interpreted language[^1]. Running an interpreted language within
another host interpreted language will necessarily be slower than just
executing the equivalent code in that host language. Python already has a
reputation for being slow, so the lower bound for our execution time is already
not looking to great, and in practice it is difficult to even get close to that
lower bound. Secondly, the reference interpreter is currently implemented as
what is known as a "tree-walk interpreter", where program execution performed
by evaluating individual nodes from the program's abstract syntax tree that was
built during program parsing. Tree-walk interpreters are really easy to make an
debug, but they are sloooooooow to execute as they involve a lot of indirect
calls that are not friendly to your instruction cache. So if Python is slow and
tree-walk interpreters are slow, then one can imagine that a tree-walk
interpreter written in Python is just about the slowest way to execute a
programming language.

### A Standard Library That Is Too Minimalist For An Ad-Hoc Scripting Language

You know what sucks? Installing packages. Every package is another opportunity
for a supply chain attack, another person who you need to trust to have the
same code quality standards as yourself, and another "thing" you need to keep
track of in your software bill of materials. I don't think it's a coincidence
that some of the best programming languages tend to have rich standard
libraries that rarely require you to reach out to external packages, and some
of the worst programming languages use `npm` to pull in packages for what
should be basic language features.

I think that a scripting language meant writing one-off scripts should have the
most juiced up batteries-included standard library possible. A nominal install
of such a scripting language should probably have, at a minimum, rich
collection types (resizable arrays, associative arrays, sets, etc.),
iterator/iteration tools, full Unicode text support with explicit support for
UTF-8, regular expressions, datetime parsing/formatting,
base64/URL-encoded/HTML-encoded text encoding/decoding, filesystem operations
(rewriting, writing, creating, deleting, and moving both files and
directories), HTTP client support, concurrency and multiprocessing
capabilities, and support for common data formats (CSV, JSON, TOML, etc.).
Ideally, these utilities should not even require an `import` statement or
equivalent.

Mellifera currently support *some* of the things listed above, but there is a
lot on that list that is missing, and probably even more things that I couldn't
think about on the spot while writing out that list. I think that Mellifera is
slowly getting there, but it is going to take a while before the language
really has all of the pieces in place for a true batteries-included experience
out of the box.

```sh
$ # this is great
$ mf -c 'println(json::encode({ .name = "Alice", .age = 42}));'
{"name": "Alice", "age": 42}
$ # this is not
$ mf -c 'println(url::encode("hello world"));'
[line 1] error: identifier `url` is not defined
```

From my experience working on [Sunder](https://github.com/ashn-dot-dev/sunder),
I can confidently say that if these sorts of tools are not easily accessible,
then it is hard to justify picking up that language for anything other than
larger projects where one might be the time to properly sort through existing
libraries or build/wrap their own dependencies as needed. For a modern
language, these things are just table stakes at this point.

## Poor Deployment And Embedding Story

How does one install Mellifera on macOS and Linux? It's actually pretty simple.

```sh
~/sources/mellifera$ python3 -m venv .venv-mellifera; . .venv-mellifera/bin/activate; python3 -m pip install -r requirements.txt >/dev/null;
(.venv-mellifera) ~/sources/mellifera$ make install >/dev/null 2>&1
(.venv-mellifera) ~/sources/mellifera$ cat >>~/.bashrc <<EOF
export MELLIFERA_HOME="$HOME/.mellifera"
if [ -e "$MELLIFERA_HOME/env" ]; then
    . "$MELLIFERA_HOME/env"
fi
EOF
(.venv-mellifera) ~/sources/mellifera$ . ~/.bashrc
~/sources/mellifera$ mf -c 'println("hello world");'
hello world
```

The install process could probably be a bit simpler, maybe with some sort of
install script or something. But even as it is now, installing an running
standalone Mellifera tooling on macOS and Linux is pretty easy.

Okay so how does one install Mellifera on Windows? Uhhh.. Well installing
Python on Windows can be a little bit of a pain, but as long as you have an
up-to-date Python install then... Oh wait Nuitka on Windows requires a C
compiler with support for C11 installed, so I guess we need to install Visual
Studio and then just... Well I guess we could use `$HOME\.mellifera` as the
default install directory, and then have user manually update their `$PATH` to
include `$HOME\.mellifera\bin` using one of the like six or seven separate ways
to do that on Windows... And then I guess we are good?

It is probably obvious from the above paragraph, but there has been essentially
zero effort put into Windows support for Mellifera. From my experience at
`$JOB` where I work almost exclusively in Windows, developing and maintaining
software on Windows kind of sucks, and supporting a Python-based project on
Windows kind of double-sucks. Like eventually I'll get around to doing it, but
right now I don't have a strong incentive to pour work into platform support
for an operating system that I know is going to be a pain.

## Footnotes
[^1]:
🤓 "Well actually, CPython *does* compile Python to bytecode before execution,
and Python *can* technically be AoT and JIT compiled." -Some Nerd Right Now
