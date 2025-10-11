Scripting with Value Semantics Even Harder Using Mellifera
==========================================================

For the past few weeks I have been working on
[Mellifera](https://github.com/ashn-dot-dev/mellifera), a new scripting
language featuring value semantics with structural equality, copy-on-write data
sharing, strong dynamic typing, explicit references with pointer-like behavior,
a lightweight nominal type system with structural protocols, built-in regular
expression support, strong iterator support with the ability to create custom
user-defined iterators, template strings with support for interpolation of
arbitrary expressions, try-catch error handling with beautiful top-level stack
traces, a batteries-included standard library filled with useful goodies for
writing ad-hoc scripts, and a ton more!

Here is a taste of Mellifera in the form of a word counting program straight
from the project's `examples` directory:

```mellifera
let help = function(writeln) {
    writeln($```
usage: {argv[0]} [file]

positional arguments:
  file          Text file to process (defaults to stdin).

optional arguments:
      --json    Output results as a JSON object.
      --top=n   Output only the top n words.
  -h, --help    Display this help text and exit.
    ```.trim());
};

let file = null;
let text = null;
let fmt = null;
let top = null;
let argi = 1;
while argi < argv.count() {
    if argv[argi] =~ r`^-+json$` {
        fmt = "json";
        argi = argi + 1;
        continue;
    }
    if argv[argi] =~ r`^-+top(=(.*))?$` {
        if re::group(2) != null {
            top = number::init(re::group(2));
        }
        else {
            argi = argi + 1;
            top = number::init(argv[argi]);
        }
        argi = argi + 1;
        continue;
    }
    if argv[argi] =~ r`^-+h(elp)?$` {
        help(println);
        exit(0);
    }

    if file != null and text != null {
        error $"multiple input files, {file} and {argv[argi]}";
    }
    file = argv[argi];
    text = fs::read(file);
    argi = argi + 1;
}
if file == null and text == null {
    file = "<stdin>";
    text = input();
}

let occurrences = Map{};
let words = re::split(text, r`\s+`)
    .iterator()
    .transform(function(word) {
        return re::replace(word.to_lower(), r`[^\w]`, "");
    })
    .filter(function(word) {
        return word.count() != 0;
    });
for word in words {
    if occurrences.contains(word) {
        occurrences[word] = occurrences[word] + 1;
        continue;
    }
    occurrences[word] = 1;
}

let ordered = occurrences
    .pairs()
    .sorted_by(function(lhs, rhs) {
        return rhs.value - lhs.value;
    })
    .iterator()
    .transform(function(pair) {
        return {
            .word = pair.key,
            .count = pair.value,
        };
    }).into_vector();

if top != null {
    ordered = ordered.slice(0, min(top, ordered.count()));
}

if fmt == "json" {
    let map = Map{};
    for x in ordered {
        map[x.word] = x.count;
    }
    println(json::encode(map));
}
else {
    for x in ordered {
        println($"{x.word} {x.count}");
    }
}
```

```sh
$ curl -s https://www.gutenberg.org/files/71/71-0.txt | mf examples/word-count.mf --top 5
the 692
to 440
and 418
of 391
a 293
$ curl -s https://www.gutenberg.org/files/71/71-0.txt | mf examples/word-count.mf --top=10 --json
{"the": 692, "to": 440, "and": 418, "of": 391, "a": 293, "it": 212, "i": 188, "in": 181, "is": 173, "not": 171}
```

Mellifera is a hard fork of another programming language project of mine called
[Lumpy](/blog/2024-09-02-scripting-with-value-semantics-using-lumpy.html) which
also presents value semantics as its principal language feature. This new
language is intended to build upon all of the best ideas from Lumpy while using
the lessons learned during the development of that project to avoid the
pitfalls that ended up making Lumpy less useful in practice than originally
envisioned. In short, Mellifera is a programming language with a much narrower
focus, more developer quality-of-life features at the language level, a smarter
architecture with less host-language-specific behavior bleeding into the
reference implementation, a more batteries-included standard library, and a
reasonable path to port the current reference implementation in Python to a
more performant language when the time is right.

This project is intended to be a slow burn where new language features and
builtins are only added as the need for them arises in other projects. I am
currently using Mellifera for the bespoke markup engine on this website[^1],
and I plan to use the language for some other small projects in the future.
However, I specifically want to avoid burnout when working on Mellifera, so
progress on the project is purposefully intended to proceed at a scenic pace.

For now, you can check out the reference implementation in Python on either the
project's [GitHub](https://github.com/ashn-dot-dev/mellifera) or
[SourceHut](https://git.sr.ht/~ashn/mellifera) pages. I hope you find some
value in programming with these value semantics! 😉

## Footnotes
[^1]:
Which now gives me access to cool effects like this {{Wave}}wavy text{{/Wave}}!
