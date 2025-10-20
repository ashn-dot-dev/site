The Mellifera Master Plan to Improve Performance and Portability
================================================================

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
functions have already been implemented in the reference interpreter. The
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
about this Mellifera reference interpreter, and about how Mellifera is
currently developed and deployed.

## The Current Mellifera Reference Interpreter

Looking at the [Mellifera
repository](https://github.com/ashn-dot-dev/mellifera/tree/df1d59a822052c3f01b641576b04a53d9f46a6d9)
on the day of this blog post publication, we see a relatively simple Python
project structure with corresponding Python-flavored [development
instructions](https://github.com/ashn-dot-dev/mellifera/blob/df1d59a822052c3f01b641576b04a53d9f46a6d9/README.md#development).
In this repository, the Mellifera interpreter is implemented within a single
Python script,
[`mf.py`](https://github.com/ashn-dot-dev/mellifera/blob/df1d59a822052c3f01b641576b04a53d9f46a6d9/mf.py).
This Python script is the thing I have been referring to as the "reference
interpreter", and is, at least at the time of writing, the authoritative
definition of the Mellifera language. That is to say, the behavior of a
Mellifera program **is** whatever the behavior of this Python script does when
executing that program.

When standalone Mellifera tooling is built via `make build` or `make install`,
the `mf.py` script is compiled into a native executable, `mf`, using
[Nuitka](https://nuitka.net/). The `mf` binary is independent from the actual
`python3` interpreter on your system, but under the hood everything is still
just Python.

```
~/sources/mellifera$ python3 mf.py -c 'println("hello world");'
hello world
~/sources/mellifera$ make build >/dev/null 2>&1
~/sources/mellifera$ ./bin/mf -c 'println("hello world");'
hello world

~/sources/mellifera$ # on macOS
~/sources/mellifera$ otool -L bin/mf
bin/mf:
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1351.0.0)
	/opt/homebrew/opt/python@3.13/Frameworks/Python.framework/Versions/3.13/Python (compatibility version 3.13.0, current version 3.13.0)
```

Look you can even do a horrifying trick at runtime by executing new Python code
from within the Mellifera itself.

```mellifera
println("This is plain ol' Mellifera code.");
println("Mellifera normally does not have a way to execute HTTP requests!");
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
Mellifera normally does not have a way to execute HTTP requests!
================================
What did one plate say to the other plate? Dinner is on me!
```

Having this singular language implementation written in Python means that
Mellifera as a whole tends to share many of the same benefits and drawbacks
seen in similar Python-based projects. Iteration times are quick, the package
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
back by what I quite frankly consider to be show-stopping usability problems
related to the portability and performance of the language and interpreter.
From my perspective, the primary challenges facing Mellifera are:

+ Unacceptably slow interpreter performance
+ A Standard library that is too minimalist for an ad-hoc scripting language
+ A Poor cross-platform deployment and embedding story

### Unacceptably Slow Interpreter Performance

I feel like scripting languages get a little bit of a pass when it comes to
runtime performance. Like, if someone is writing a one-off throwaway script in
Python, then chances are they probably do not care that much if it executes
100x slower than the equivalent code written in a language like C++ or Rust. Of
course there will always be outliers like LuaJIT and PyPy which execute their
respective scripting languages blazingly fast™, but in general it seems like
there is this trade-off between convenience and performance where scripting
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
in which their interpreters are actively running. Compare that to Mellifera
where it takes almost a tenth of a second to print "hello world" on a fairly
powerful machine. That Mellifera startup and execution time is a huge red flag!
And it gets even worse with larger programs containing more compute-heavy
operations. Consider the example word counting program featured in the
Mellifera project README. Executing this program on a relatively small input is
over 250x slower than running a similar command builtin from programs strung
together in Shell.

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
executing the equivalent code in that host language from the start. Python
already has a reputation for being slow, so the lower bound for our execution
time is already not looking too great, and in practice it is difficult to even
get close to that lower bound. Secondly, the reference interpreter is currently
implemented as what is known as a "tree-walk interpreter", where program
execution performed by evaluating individual nodes directly from the program's
abstract syntax tree. Tree-walk interpreters are really easy to build and
debug, but they are sloooooooow to execute, as evaluating a program in a
tree-walk interpreter involves a lot of indirect calls that are not friendly to
your CPU's instruction cache. So if Python is slow and tree-walk interpreters
are slow, then one can imagine that a tree-walk interpreter written in Python
is just about the slowest way to execute a programming language.

### A Standard Library That Is Too Minimalist

You know what sucks? Installing packages. Every package is another opportunity
for a supply chain attack, another person who you need to trust to have the
same code quality standards as yourself, and another item you need to keep
track of in your software bill of materials. I don't think it's a coincidence
that some of the best programming languages tend to have rich standard
libraries that rarely require you to reach out to external packages, and some
of the worst programming languages use `npm` to pull in a million tiny packages
for what should be basic language functionality.

I think that a scripting language designed for writing one-off scripts should
have the most juiced-up batteries-included standard library possible. A nominal
install of such a scripting language should probably have, at a minimum, rich
collection types (resizable arrays, associative arrays, sets, etc.),
iterator/iteration tools, full Unicode text support with explicit support for
UTF-8, regular expressions, datetime parsing/formatting, base64/URL/HTML text
encoding/decoding, filesystem operations (reading, writing, creating, deleting,
and moving both files and directories), HTTP client support, concurrency and
multiprocessing capabilities, and support for common data formats (CSV, JSON,
TOML, etc.). Ideally, these utilities should not even require an `import`
statement or equivalent.

Mellifera currently supports *some* of the items listed above, but there is a
lot on that list which is still missing, and there are probably even more
things that didn't come to mind while writing out that list just now. I think
that Mellifera is slowly getting there, but it is going to take a while before
the language really has all of the pieces in place for a true out-of-the-box
batteries-included experience.

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
larger projects where one *might* have the time to properly sort through
existing libraries or build/wrap their own dependencies as needed. For a modern
scripting language, these things are just table stakes.

But this seems like a solvable problem right? Like, if Melliera is missing CSV
support, then why not just add a CSV library to the interpreter base
environment? Look Python even has [CSV
support](https://docs.python.org/3/library/csv.html) built into its standard
library! Well, dear reader, the problem of spotty standard library support is
less of a technical limitation, and more of deliberate choice to avoid
implementation specific behavior.

See, I knew that this Mellifera reference interpreter would probably not scale,
and that eventually there might be some form of a rewrite or alternate
implementation. So during the development of Mellifera, I took great care to
make sure that Mellifera did not rely too heavily on the underlying behavior
of the Python language. It would be really unfortunate if a second Mellifera
implementation was developed in a different host language and it turned out
that parts of Mellifera could not be implemented in that second host language
due to differences in the two host language implementations. A great example of
this would be something like regular expressions, where the syntax and
semantics of [Python's `re` library](https://docs.python.org/3/library/re.html)
is different from the syntax and semantics of [C++'s `std::regex`
type](https://en.cppreference.com/w/cpp/regex/basic_regex.html), which is
different from the syntax and semantics of [Rust's blessed `regex`
crate](https://github.com/rust-lang/regex). If Mellifera just used Python's
builtin `re` library for Mellifera regular expressions, then implementing
Mellifera in another host language would mean that the validity of any given
regular expression in a Mellifera script would depend on the host language of
one's Mellifera interpreter, something that should (at least ideally) be a
hidden implementation detail.

So a major reason that these batteries-included library features are currently
missing from Mellifera is that I have had to be cautious as a language author
to not let anything implementation specific slip by. With limited free time to
work on personal projects like this, and without a second implementation of
Mellifera to verify that core language and standard library additions are
indeed implementation agnostic, it is difficult to make headway at a reasonable
pace with a high degree confidence.

## A Poor Cross-Platform Deployment And Embedding Story

How does one install Mellifera on macOS and Linux? It's actually pretty simple.

```sh
~/sources/mellifera$ python3 -m venv .venv-mellifera; . .venv-mellifera/bin/activate; python3 -m pip install -r requirements.txt >/dev/null
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
install script or something. But even as it is now, installing and running
standalone Mellifera tooling on macOS and Linux is not too difficult.

Okay so how does one install Mellifera on Windows? Uhhh.. Well installing
Python on Windows can be a little bit of a pain, but as long as you have an
up-to-date Python install then... Oh wait Nuitka on Windows requires a C
compiler with support for C11 installed, so I guess we need to install Visual
Studio and then just... Well I guess we could use `$HOME\.mellifera` as the
default install directory, and then have the user manually update their `$PATH`
to include `$HOME\.mellifera\bin` using one of the like six or seven separate
ways to do that on Windows... And then *maybe* we are good?

If it is not obvious from the above paragraph, I should be clear that there has
been essentially zero effort put into Windows support for Mellifera. From my
experience at `$CURRENT_JOB` where I work almost exclusively in Windows,
developing and maintaining software on Windows kind of sucks, and supporting a
Python-based project on Windows kind of double-sucks. Like, eventually I could
get around to supporting Windows, but right now I don't have a strong incentive
to pour work into platform support for an operating system that I know is going
to be a pain.

And of course we can't forget the browser! I can't imagine that many people
would have a need for an ad-hoc scripting language in the browser when
JavaScript already exists, but it would be nice to have some sort of
browser-based interpreter for tutorial materials. Additionally, a language like
Mellifera *might* be useful for embedded scripting, and I think it is
reasonable to expect that one could compile a game engine to run in the browser
via WebAssembly with Mellifera embedded as the scripting interface to the
engine. Python *can* run in the browser with WebAssembly, but given the
existing performance problems one encounters when running Mellifera natively, I
find it difficult to believe that the Mellifera reference interpreter perform
at an acceptable speed in the browser.

## Musing On The {{Wave}}Mellifera Master Plan{{/Wave}} To Improve Performance and Portability

To recap, the current Mellifera reference interpreter is unacceptably slow, is
missing functionality that is critical for a modern ad-hoc scripting language,
and has a cross-platform deployment and embedding story that is lacking. As it
exists today, Mellifera is an interesting language that is held back by
architectural and implementation-specific setbacks stemming from having a
single Python-based language implementation. I want Mellifera to thrive. I want
this language to bring joy to those using it. I want quick edit-debug cycles. I
want a rich standard library. I want to be able to use Mellifera on the
desktop, on a phone, in a web-browser, and I want the deployment strategy for
all of those platforms to be friction-free. It certainly seems like we need
another implementation of Mellifera in a different host language, but how do we
get there? How does one evolve a language like Mellifera without accidentally
introducing breaking changes? And how do we guarantee that these two
implementations of Mellifera will not diverge in the future? Enter the
**{{Wave}}Mellifera Master Plan{{/Wave}}**!

The **{{Wave}}Mellifera Master Plan{{/Wave}}** is a long-running effort to port
the Mellifera interpreter from Python to Go, keeping the Python-based
implementation around as the slower yet simpler reference interpreter, and
having this newer Go-based implementation serve as the production interpreter
that is used for standalone scripting and embedding. Go is a reasonably
performant compiled language with built-in garbage collection and strong
multi-platform support. Go is a boring language with a comprehensive standard
library and a strong package ecosystem that follows standard conventions. And
although Go and Python differ in their approaches to idea expression and error
handling, there is enough overlap between typed Python and Pythonic-looking-Go
that it should be possible to port the existing tree-walk interpreter from
Python to Go while keeping the same shape of the code.

In the **{{Wave}}Mellifera Master Plan{{/Wave}}**, development of the second
Go-based interpreter will be broken down into distinct phases.

1. Add support for token dumps in the Python-based interpreter.
2. Write a Mellifera lexer in the Go-based interpreter, adding support for
   token dumps from the start to ensure that the Go-based interpreter exactly
   matches the lexing behavior of the Python-based interpreter.
3. Add support for abstract syntax tree dumps in the Python-based interpreter.
4. Write a Mellifera parser in the Go-based interpreter, adding support for
   abstract syntax tree dumps from the start to ensure that the Go-based
   interpreter exactly matches the parsing behavior of the Python-based
   interpreter.
5. Implement a tree-walk evaluator to the Go-based interpreter, ensuring that
   the evaluation behavior of Mellifera programs in the Go-based interpreter
   exactly matches the evaluation behavior of those same programs in the
   Python-based interpreter.
6. Implement all built-in Mellifera functions from the Python-based interpreter
   to the Go-based interpreter, using the existing test suite to verify that
   the behavior of all built-in functions exactly match between the two
   implementations.
7. Flesh out the standard library with the batteries-included functionality
   expected from a modern ad-hoc scripting language, using the two separate
   implementations to ensure that the behavior of the standard library is not
   tied to the underlying behavior of either host language or implementation.
8. Add a bytecode interpreter mode and bytecode compilation to the Go-based
   interpreter, ideally bringing the performance of the Go-based interpreter
   more inline with similar bytecode-based interpreters such as CPython or the
   Lua VM.

The idea with each one of these phases is to do the minimum amount of work
required in order to verify that the Python-based interpreter and Go-based
interpreter exhibit the same behavior. Instead of doing a whole rewrite from
scratch and comparing language behavior *after* the new interpreter is
finished, we make incremental bits of progress where we can check our work
along the way. If we can verify matching behavior after every bit of
incremental progress, then we should (in theory) end up with a compatible
language implementation by the time the tree-walk evaluator and all built-in
functions are supported in the Go implementation.

The phases of the **{{Wave}}Mellifera Master Plan{{/Wave}}** appear in the same
order that one would use to implement a new language from scratch: lexing,
parsing, tree-walk evaluation, add standard library enhancements, optimize the
implementation. So from a development perspective, work on the Go-based
interpreter will look and feel a lot like working on the original Mellifera
implementation with added pauses to verify behavioral compatibility between the
Python and Go implementations.

### Behavioral Compatibility Verification In Practice

When we talk about verifying the behavioral compatibility of the Python-based
interpreter and Go-based interpreter, what does that actually mean? Let's just
focus on the **{{Wave}}Mellifera Master Plan{{/Wave}}** steps 1 and 2: adding
token dump support to the Python-based interpreter and writing a Mellifera
lexer in Go with token dump support. In the Python-based interpreter, we would
add a flag `--dump-tokens` which will tell the interpreter to output the tokens
lexed from a specified input file.

```sh
$ cat examples/hello-world.mf
println("Hello, world!");
$ python3 mf.py --dump-tokens examples/hello-world.mf
println
(
"Hello, world!"
)
;
```

This gives us a text representation of what the lexer sees when it parses a
source file. Since the Python-based implementation and Go-based implementation
should parse the same list of tokens, running the Go-based implementation with
the `--dump-tokens` flag should produce the same output.

```sh
$ go run mf.go --dump-tokens examples/hello-world.mf
println
(
"Hello, world!"
)
;
```

We can `diff` the output of both interpreters run with the `--dump-tokens`
flag, and as long as there is no difference between those two outputs, then we
know that the Go lexer is equivalent to the Python lexer for that particular
input file. We can test this on one large `all-of-the-tokens.mf` file where we
try and lex every kind of token and exercise all of the known edge cases in the
lexer, but we also have the benefit of over two hundred test files in the
Mellifera [`tests`
directory](https://github.com/ashn-dot-dev/mellifera/tree/df1d59a822052c3f01b641576b04a53d9f46a6d9/tests)
and several example programs in the [`examples`
directory](https://github.com/ashn-dot-dev/mellifera/tree/df1d59a822052c3f01b641576b04a53d9f46a6d9/examples),
all of which can serve as additional test cases for comparing the token dump
outputs. As long as there is no difference between the output of the
Python-based interpreter token dump and the Go-based interpreter token dump for
all of those test cases, then we can be pretty confident that the Go lexer is
equivalent to the Python lexer for the entirety of the language. As a bonus,
using existing tests for this purpose will also allow us to verify that the two
implementations produce the same error messages when a lexing error is
encountered, since the `tests` directory contains a bunch of cases that already
check for specific lexing and parse errors that are produced by the reference
interpreter.

```sh
$ # test for a specific parse error from an ambiguous `{}` collection literal
$ cat tests/error-ambiguous-map-or-set.test.mf
{};
################################################################################
# [error-ambiguous-map-or-set.test.mf, line 1] ambiguous empty map or set

$ python3 mf.py tests/error-ambiguous-map-or-set.test.mf
[tests/error-ambiguous-map-or-set.test.mf, line 1] ambiguous empty map or set

$ go run mf.go tests/error-ambiguous-map-or-set.test.mf
[tests/error-ambiguous-map-or-set.test.mf, line 1] ambiguous empty map or set
```

This is actually very similar to how the Mellifera test suite is normally run.
I have a [whole blog
post](/blog/2024-11-04-dead-simple-testing-for-programming-language-projects.html)
on how I use golden testing in my programming language projects, but in short,
a `.test.mf` file contains a Mellifera program, followed by an optional line of
comment characters, followed by the expected output for that program, all in
one file. This is the testing technique I used for
[Sunder](https://github.com/ashn-dot-dev/sunder/blob/main/bin/sunder-test),
[Lumpy](https://github.com/ashn-dot-dev/lumpy/blob/main/bin/lumpy-test), and
now
[Mellifera](https://github.com/ashn-dot-dev/mellifera/blob/df1d59a822052c3f01b641576b04a53d9f46a6d9/bin/mf-test).
When you run a test with Mellifera's `mf-test`, this golden testing technique
is performed by `diff`ing the actual output of the provided test file with the
expected output from that same test file.

```sh
$ cat tests/error-ambiguous-map-or-set.test.mf
{};
################################################################################
# [error-ambiguous-map-or-set.test.mf, line 1] ambiguous empty map or set

$ (cd tests && mf error-ambiguous-map-or-set.test.mf)
[error-ambiguous-map-or-set.test.mf, line 1] ambiguous empty map or set

$ mf-test tests/error-ambiguous-map-or-set.test.mf
[= TEST tests/error-ambiguous-map-or-set.test.mf =]
[= PASS =]
TESTS RUN => 1
FAILURES  => 0
```

These incremental improvements are essentially applying the same golden testing
principle to lexing, but using the token dump from the reference interpreter as
the "expected" output and the token dump from the Go-based interpreter as the
"actual" output. We do this implementation comparison for the lexer with a
`--dump-tokens` flag, then later we do the same for the abstract syntax tree
with a `--dump-ast` flag, and finally we can use the test suite directly to
test the Go-based implementation with `mf-test` once the tree-walk evaluator
and built-in functions are added to the Go-based interpreter.

I have actually done this sort of thing before at `$PREVIOUS_JOB` when I worked
as a compiler engineer. There was a massive project to refactor a chunk of the
our compiler's parser, and as part of that refactor I wanted to guarantee that
the updated parser would produce byte-for-byte identical output to the original
parser as a way to ensure that the refactor did not introduce any behavioral
changes. In order to make that happen, I wrote a bunch of custom tooling that
allowed me to built two versions of the compiler, one with the original parser
enabled and one with the updated parser enabled, and then run those two
compiler builds over a sample of 100k+ programs, similar to what we would be
doing by `diff`ing the `--dump-tokens` output between the Python and Go
implementations. In the case of `$PREVIOUS_JOB`, the high degree of confidence
gained from having such robust coverage allowed me to ship the feature with
assurance that nothing would break in production[^2]. And in the case of this
project, I have a similar confidence that we would be able to find most
incompatibilities during development, or at the very least have a framework in
place to automatically verify that any incompatibilities which happen to be
overlooked will receive proper validation once a test case with that
incompatibility is added to the Mellifera repository as a `.test.mf` test file.

## Implementation Timeline and Conclusions

Alright so we have the **{{Wave}}Mellifera Master Plan{{/Wave}}** in place as
our path forward to building a separate Mellifera implementation that should be
much more performant than the existing reference interpreter. When everything
is complete, we will have two distinct implementations of Mellifera that will
allow us to add new language features and built-in functions with confidence
that those features and functions do not accidentally rely on
implementation-specific details of the interpreter host languages. And due to
the nicer out-of-the box cross-platform support that Go provides, building and
deploying in environments like Windows and Wasm will be less arduous of a task.

This all sounds great in the abstract, however, it should be noted that the
**{{Wave}}Mellifera Master Plan{{/Wave}}** is actually a ton of work. I am a
software developer with a full time job, a house to maintain, a lovely partner
who I would like to spend time with, and three attention-hungry cats who need
daily care and stimulation, so I am not exactly swimming in free time at the
moment. Not to mention that Mellifera is not even my main side-project right
now! I am actually trying to chill out a bit and focus on this website's
[recipe section](/recipes.html), and a big chunk of the reason I have been
dedicating so much time to Mellifera over the past month is because I wanted to
use my own bespoke scripting language to implement the transpiler for my
bespoke tag-based markup language that is used to generate the blog entry and
recipe pages in the bestpoke static site generator that builds this website. So
in many ways the **{{Wave}}Mellifera Master Plan{{/Wave}}** is a side quest of
a side quest that is firmly in the category of "slow burn project that only
receives updates when whenever I happen to feel the inspiration to work on it".

This is all to say that some of the early steps in the **{{Wave}}Mellifera
Master Plan{{/Wave}}** may take a month. They may take a quarter. They may take
a year or more! However, when I *do* get around to working on this project, I
am confident that the **{{Wave}}Mellifera Master Plan{{/Wave}}** is going to
provide a strong path towards a more performant, more batteries-included, more
portable language that will only get better with time.

## Footnotes
[^1]:
🤓 "Well actually, CPython *does* compile Python to bytecode before execution,
and Python *can* technically be AoT and JIT compiled." -Some Nerd Right Now

[^2]:
I kept that "enable the original parser" toggle in the compiler when the
feature was shipped just in case we encountered an unforeseen problem in
production, but there ended up being zero problems after deployment. As far as
I am aware, the updated parsing code (and a bunch of the other systems I wrote
in conjunction with the updated parsing code) still receive praise at that
company.
