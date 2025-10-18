The Mellifera Master Plan to Maximize Portability and Performance
=================================================================

I have been developing [Mellifera](https://github.com/ashn-dot-dev/mellifera)
for about a month now, during which time the language has shaped up into
something that I find quite pleasent to work with. My brain naturally thinks
about code and data in terms of value semantics, so it is really lovely to have
a scripting language where value semantics are at the heart of the programming
model.

```mellifera
let a = ["foo", "bar", "baz"];
let b = a; # b is a copy of a
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
functions have already been implemented in the [reference
interpreter](https://github.com/ashn-dot-dev/mellifera/blob/main/mf.py), and
the language is now at a point where one could reasonably use it for the use it
for the same flavor of ad-hoc command line tools that would typically be
handled by something like Perl or Unix tools strung together in POSIX shell.

```sh
$ # some random selection of English words
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
