Dead Simple Testing for Programming Language Projects
=====================================================

## Introduction

Say you are working on a new programming language project. The language is just
getting out of the proof-of-concept phase, and you want to set up some form of
automated testing for the compiler and/or interpreter. What would a testing
tool for a programming language even look like? How do you test individual
language features that may be mutually dependent on other language features?
What can you do to make writing new tests easy? What can you do to make
updating existing tests as frictionless as possible? How do you write a test
for a syntax error? How do you write a test for something in the language's
standard library?

In this article we will look at a simple strategy for testing programming
languages and their associated compilers/interpreters. We will examine the test
tooling that I have successfully used in two programming language projects,
[Sunder](https://github.com/ashn-dot-dev/sunder) and
[Lumpy](https://github.com/ashn-dot-dev/lumpy), in order to learn how that
tooling works and why it is so effective. By the end of this post, you should
have answers to all of the questions above, as well as a firm understanding of
how to build something like this for your own programming language projects.

## A Tour of the Test Tool

With the introductions out the way, let's dive right into how Sunder and Lumpy
are tested. Both repositories contain a variation on the same basic shell
script. This script, named
[`sunder-test`](https://github.com/ashn-dot-dev/sunder/blob/main/bin/sunder-test)
in the Sunder repository and
[`lumpy-test`](https://github.com/ashn-dot-dev/lumpy/blob/main/bin/lumpy-test)
in the Lumpy repository, is what gets run when each project's `make check`
target is executed. The `{language}-test` scripts also serve as the standard
test utilities for each language, so when writing a third-party library for
either language, one would also invoke `{language}-test` for that library's
tests. Given that these scripts are almost identical, we will primarily focus
on the `lumpy-test` script for the remainder of this article.

Alright, so what does the `lumpy-test` script actually do, and what do the
tests executed by this script look like? In short, the `lumpy-test` script
performs a variation of [golden
testing](https://ro-che.info/articles/2017-12-04-golden-tests) on a provided
test file. A test file (identified by the file extension `.test.lumpy`)
contains a Lumpy program alongside the expected output of that program embedded
in comments at the end of the file.

```
let x = 123 + 456;
println("123 + 456 is " + string(x));
################################################################################
# 123 + 456 is 579
```

In the above test, the lines:

```
let x = 123 + 456;
println("123 + 456 is " + string(x));
```

contain of the actual program being tested. In this case the test is adding two
numbers and printing out the result of that addition to the standard output
stream. Then, a long line of comment characters separates the program section
of the test from the expected output section of the test. Finally, the expected
output section of the test:

```
# 123 + 456 is 579
```

indicates that the expected output of this test should be the text `"123 + 456
is 579"`. When we run this test with the lumpy interpreter, we indeed see that
this text is printed:

```sh
$ lumpy add.test.lumpy
123 + 456 is 579
```

And when we run this test with `lumpy-test`, we see that the test tool agrees
that the expected output of the test matches the actual output of the test:

```sh
$ lumpy-test add.test.lumpy
[= TEST add.test.lumpy =]
[= PASS =]
TESTS RUN => 1
FAILURES  => 0
```

If the expected output of a test does *not* match the actual output of a test,
then the test tool will show a `diff` of the expected and actual output, and
indicate that the test has failed.

```sh
$ cat mul.test.lumpy
let x = 123 - 456; # purposefully wrong
println("123 * 456 is " + string(x));
################################################################################
# 123 * 456 is 56088
$ lumpy-test mul.test.lumpy
[= TEST mul.test.lumpy =]
1c1
< 123 * 456 is 56088
---
> 123 * 456 is -333
[= FAIL =]
TESTS RUN => 1
FAILURES  => 1
```

Multiple tests can be passed as command line arguments to `lumpy-test`:

```sh
$ lumpy-test add.test.lumpy mul.test.lumpy
[= TEST add.test.lumpy =]
[= PASS =]
[= TEST mul.test.lumpy =]
1c1
< 123 * 456 is 56088
---
> 123 * 456 is -333
[= FAIL =]
TESTS RUN => 2
FAILURES  => 1
```

And if no command line arguments are passed to `lumpy-test` then the script
will use `find` to locate all files ending with `.test.lumpy` and run those
tests automatically:

```sh
$ ls
add.test.lumpy	mul.test.lumpy
$ lumpy-test
[= TEST ./add.test.lumpy =]
[= PASS =]
[= TEST ./mul.test.lumpy =]
1c1
< 123 * 456 is 56088
---
> 123 * 456 is -333
[= FAIL =]
TESTS RUN => 2
FAILURES  => 1
```

Some really important things that I want to emphasize are (1) test files are
just regular programs and (2) the expected output of the test is *right* next
to the code that is supposed to generate that expected output. Keeping test
files as regular programs means that debugging a test is as easy as executing
the test over and over in an edit-and-run cycle until the output printed to
your terminal and the expected test output match. When I see that my newly
written `mul.test.lumpy` test is failing, I can look at just the actual output
of the test by running the interpreter on that test file:

```
$ lumpy-test
[= TEST ./add.test.lumpy =]
[= PASS =]
[= TEST ./mul.test.lumpy =]
1c1
< 123 * 456 is 56088
---
> 123 * 456 is -333
[= FAIL =]
TESTS RUN => 2
FAILURES  => 1
$ # oops! something is wrong!
$ lumpy mul.test.lumpy
123 * 456 is -333
```

Walking through and rubber-duck-debugging the test output as part of this
edit-and-run cycle is a lot easier when the expected output is in the same file
as the test. No switching between tabs in my editor, no separate `.expected`
file that I need to keep in sync with the actual test program, and no
additional test framework boilerplate. While working on Sunder and Lumpy, my
workflow when updating the compiler/interpreter/standard-libary tests tends to
be:

1. Write my changeset
2. Run the full test suite with `{language}-test`
3. If a test is failing, run that test individually with `{language}-test {that-test}`
4. Make additional edits attempting to address the problem
5. Run the program standalone and check the behavior changes
6. Goto 4 until the test is in the shape I would expect
7. Verify the individual test passes when run with `{language}-test {that-test}`
8. Goto 2 until all tests are passing
9. Commit my changeset

This workflow is super easy with the way that the test tooling is set up, and
there is very little cognitive load I have to keep track of outside the
singular test file I am working with.

## Digging Into the Implementation Details

Okay so we have seen what it is like to work with the test tooling. Let us now
take a look at the actual shell script itself and find out why I consider this
testing method to be so "dead simple". At the time of writing, the entirety of
`lumpy-test` is as follows:

```sh
#!/bin/sh
set -e
export LC_ALL=C

PROGNAME=$(basename "$0")
usage() {
    cat <<EOF
Usage: ${PROGNAME} [FILE...]

Options:
  -h        Display usage information and exit.
EOF
}

for arg in "$@"; do
case "${arg}" in
    -h)
        usage
        exit 0
        ;;
    *)
        ;;
esac
done

if [ -z "${LUMPY_HOME}" ]; then
    LUMPY_HOME=$(pwd)
fi

TESTSRUN=0
FAILURES=0

test() {
    TEST="$1"

    echo "[= TEST ${TEST} =]"

    set +e
    RECEIVED=$(\
        cd "$(dirname "${TEST}")" 2>&1 && \
        python3 "${LUMPY_HOME}/lumpy.py" "$(basename "${TEST}")" 2>&1)
    set -e

    EXPECTED=$(\
        sed -n '/^########\(#\)*/,$p' "${TEST}" |\
        sed '1d' |\
        sed -E 's/^#[[:space:]]?//g')

    if [ "${EXPECTED}" = "${RECEIVED}" ]; then
        echo '[= PASS =]'
    else
        TMPDIR=$(mktemp -d)
        trap '{ rm -rf -- "${TMPDIR}"; }' EXIT
        printf '%s\n' "${EXPECTED}" >"${TMPDIR}/expected"
        printf '%s\n' "${RECEIVED}" >"${TMPDIR}/received"
        diff "${TMPDIR}/expected" "${TMPDIR}/received" || true
        echo '[= FAIL =]'
        FAILURES=$((FAILURES + 1))
    fi
    TESTSRUN=$((TESTSRUN + 1))
}

TESTS= # empty
if [ "$#" -ne 0 ]; then
    for arg in "$@"; do
        if [ -d "$(realpath ${arg})" ]; then
            FILES=$(find "$(realpath ${arg})" -name '*.test.lumpy' | sort)
            TESTS=$(echo "${TESTS}" "${FILES}")
        else
            TESTS=$(echo "${TESTS}" "${arg}")
        fi
    done
else
    TESTS=$(find . -name '*.test.lumpy' | sort)
fi

for t in ${TESTS}; do
    test "${t}"
done

echo "TESTS RUN => ${TESTSRUN}"
echo "FAILURES  => ${FAILURES}"

[ "${FAILURES}" -eq 0 ] || exit 1
```

That is 84 lines of fairly mundane shell. There is not anything particularly
novel happening here, and I am fairly confident that an equivalent or better
version of this script could be built in under a day. I wrote this test tooling
in POSIX shell since `find`, `sed`, and `diff` are all easily accessible on
modern Unix systems, but it is not difficult to imagine a version of this
tooling written in another language such as Python or Rust.

The meat of this script is found in the `test` function. This function takes a
relative filepath as an argument, runs the program contained in that file, and
compares the expected output of that program to the actual output generated
when the program was run, signaling a single test failure if the expected and
actual output do not match.

The actual program output is captured though the assignment to `RECEIVED`:

```sh
RECEIVED=$(\
    cd "$(dirname "${TEST}")" 2>&1 && \
    python3 "${LUMPY_HOME}/lumpy.py" "$(basename "${TEST}")" 2>&1)
```

Notice that we `cd` into the directory containing the test *before* running
that test. Some tests will have additional data that needs to be accessed
relative to the location of the test file, and we want to make sure that
relative pathing will behave the same regardless of the directory from which
the testing tool is invoked. An example of this in practice would be something
like an [Advent of Code](https://adventofcode.com/) problem, where a test
program such as `day-xx.test.lumpy` might want to read the text file
`day-xx.input` located in the same directory as the test. It is a lot more
convenient to write:

```
let text = fs::read("day-xx.input");
```

than it is to stitch together a path relative to the test file with:

```
let text = fs::read(module::directory + "/day-xx.input");
```

Once we have the actual program output, we run this funky `sed` chain to
extract out the expected program output:

```sh
EXPECTED=$(\
    sed -n '/^########\(#\)*/,$p' "${TEST}" |\
    sed '1d' |\
    sed -E 's/^#[[:space:]]?//g')
```

Here we look for a line beginning with eight or more `#` characters. Both
Sunder and Lumpy have single-line comments that begin with `#`, and a different
language would require substiting `#` with the appropriate single-line comment
character(s). We then remove that line of `#` characters, and strip the leading
`#` characters from the remaining text of the expected output section. We also
remove an optional space character just after each `#` within the expected
output section so that a line such as `# 123 + 456 is 579` will be treated the
same as the line `#123 + 456 is 579`. In my opinion, that extra space makes it
significantly easier to read the expected output section when you have been
debugging and testing all day. Just to show this `sed` chain in action, let us
take a look at what happens when we run this command on our example test from
earlier:

```sh
$ cat add.test.lumpy
let x = 123 + 456;
println("123 + 456 is " + string(x));
################################################################################
# 123 + 456 is 579
$ sed -n '/^########\(#\)*/,$p' add.test.lumpy |\
  sed '1d' |\
  sed -E 's/^#[[:space:]]?//g'
123 + 456 is 579
```

If our actual output does not match the expected output, then we `diff` the
actual and expected output and display that result to the user:

```sh
TMPDIR=$(mktemp -d)
trap '{ rm -rf -- "${TMPDIR}"; }' EXIT
printf '%s\n' "${EXPECTED}" >"${TMPDIR}/expected"
printf '%s\n' "${RECEIVED}" >"${TMPDIR}/received"
diff "${TMPDIR}/expected" "${TMPDIR}/received" || true
```

And that is pretty much everything. We have the printing of the `TEST` and the
`PASS` or `FAIL` lines which are primarily intended to make reading failing
test diffs easier. We perform some bookkeeping, incriminating the total number
of tests run every time the `test` function is executed, and incrementing the
number of failing tests right after we emit a failing test diff. But that is
about it. The script is pretty simple, and it would not be too difficult to
re-write, extend, or customize this script for another programming language
project.

## Some More Neat Stuff

This type of golden testing strategy comes with a bunch of extra benefits that
are particularly nice when working on compilers and interpreters.

**Running each test independently adds a process-level isolation between
tests!** Program state from the invocation of the compiler/interpreter used for
test A will not affect the program state during the invocation of the
compiler/interpreter used for test B. Artifacts produced by already-run tests
(e.g. files left on the filesystem) *will* still be visible to future tests,
but (in my experience) there are very few instances where that ever actually
matters.

**Compilers and interpreters don't care about individual language features!**
In the introduction section, I asked how one would test language features that
are mutually dependent on each other. Well from the perspective of the compiler
or interpreter there is only one language, and if two language features are
dependent on each other, then you do not *need* to figure out how to test them
separately. Just throw them together in one or more tests and make sure you get
good coverage over the surface area of both features. There is no dependency
injection or [friend](https://en.cppreference.com/w/cpp/language/friend)
shenanigans required to make testing easier, as the entry point to the
compiler/interpreter packages the entire language together for every test
invocation.

**Warnings and errors are captured as test output!** It is trivial to write
tests for lexical, syntactical, and semantic errors when diagnostic messages
are picked up as "actual program output" by the testing tool. Take a look at
this example from the Sunder project repository verifying that the correct
error message is emitted when an invalid number of arguments are passed to a
function:

```sh
$ cat tests/error-call-invalid-argument-count.test.sunder
func foo(a_: usize, b_: ssize, c_: bool) ssize {
    return 123s;
}

func main() void {
    foo(1u, 2s);
}
################################################################################
# [error-call-invalid-argument-count.test.sunder:6] error: function with type `func(usize, ssize, bool) ssize` expects 3 argument(s) (2 provided)
#     foo(1u, 2s);
#        ^
$ sunder-run tests/error-call-invalid-argument-count.test.sunder
[tests/error-call-invalid-argument-count.test.sunder:6] error: function with type `func(usize, ssize, bool) ssize` expects 3 argument(s) (2 provided)
    foo(1u, 2s);
       ^
$ sunder-test tests/error-call-invalid-argument-count.test.sunder
[= TEST tests/error-call-invalid-argument-count.test.sunder =]
[= PASS =]
TESTS RUN => 1
FAILURES  => 0
```

**The core language and standard library use the same testing tool!** Once
again, the testing tool only cares about actual vs expected program output.
From the perspective of the tool, the core language, builtins, standard
library, and even external libraries are all the same thing. Run program, get
output. This is refreshingly straightforward, and in practice I have found that
that writing and updating tests requires very little mental context switching.
It is difficult to express how nice it is to know that opening any test file
will present you with a normal program.

## Some Less Neat Stuff

Okay so I am a huge fan of this approach to golden testing, and I have no
intentions to deviate from this strategy in future programming language
projects. That being said, this methodology is not without its issues.

**Golden testing in general requires deterministic output, and if a program's
output is non-deterministic, then this kind of testing is useless!** For
example, stringified references in Lumpy contain the hex-encoded address of the
referenced object in the stringified text. The same program will produce a
different stringified address from run to run, so it would be difficult to
write a test validating the non-deterministic behavior of that stringification:

```sh
$ cat reference.lumpy
println(123.&);
$ lumpy reference.lumpy
reference@0x100ea7200
$ lumpy reference.lumpy
reference@0x100f97200
```

**Unrelated changes can break exist tests!** Say I wanted to add a new warning
to the Sunder compiler that will inform a user when they have a function call
with an unused return value. The patch for it is pretty simple, but there are
currently hundreds of tests (and a significant portion of the standard library)
containing call-expressions with (purposefully) unused return values. This
small change would require me to update all of those tests even though they do
not have a whole lot to do with the actual changeset. This is a form of spooky
action at a distance, and is less-than-ideal in my opinion.

```sh
~/sources/sunder[main !]$ git rev-parse HEAD
0a3595e264b0830a43285ea1c2cfd4f267673f82
~/sources/sunder[main !]$ git diff
diff --git a/resolve.c b/resolve.c
index ff8e091..9c0e5f1 100644
--- a/resolve.c
+++ b/resolve.c
@@ -3964,6 +3964,13 @@ resolve_stmt_expr(struct resolver* resolver, struct cst_stmt const* stmt)
     }
     struct stmt* const resolved = stmt_new_expr(stmt->location, expr);

+    if (expr->kind == EXPR_CALL && expr->type->kind != TYPE_VOID) {
+        warning(
+            expr->location,
+            "unused return value of type `%s`",
+            expr->type->name);
+    }
+
     freeze(resolved);
     return resolved;
 }
~/sources/sunder[main !]$ make check 2>&1 | tail -n3
TESTS RUN => 550
FAILURES  => 222
make: *** [check] Error 1
```

## Conclusion

Despite some of the limitations, I have found this flavor of golden testing to
be the most effective way to test my programming languages. The tooling shown
in this article is easy to build from scratch, is pleasant to work with, and
ends up scaling reasonably well. I have spent a good chunk of the last four
years working on programming languages that utilize this tooling, and I have
nothing but good things to say about the testing experience. I encourage
prospective language authors to experiment with this technique and remix it to
suit the needs of your project. Cheers!
