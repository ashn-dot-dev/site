Scripting with Value Semantics Using Lumpy
==========================================

For the past few weeks I have had a blast working on,
[Lumpy](https://github.com/ashn-dot-dev/lumpy), a new scripting language
featuring value semantics as the principal language feature.

For the uninitiated, **value semantics** in a programming language mean that
assignment operations will copy the contents or "value" of an object when
performing the assignment. After an assignment statement `a = b`, variables `a`
and `b` will contain separate copies of the same value. This is in contrast
**reference semantics** in which the assignment statement `a = b` will leave
`a` and `b` referencing the same underlying object.

The concept of value semantics is probably familiar to anyone who has
experience with C++, where value semantics are achieved via the use of copy
constructors and/or an overloaded assignment operator.

```
// test.cpp
#include <iostream>
#include <string>
#include <vector>

int main() {
    auto a = std::vector<std::string>{"foo", "bar", "baz"};
    auto b = a; // b becomes a *copy* of a
    a.push_back("lasagne"); // modification of a does not affect b

    std::cout << "a:";
    for (auto element: a) {
        std::cout << " " << element;
    }
    std::cout << "\n";

    std::cout << "b:";
    for (auto element: b) {
        std::cout << " " << element;
    }
    std::cout << "\n";
}
```

```
$ clang++ -std=c++11 test.cpp && ./a.out
a: foo bar baz lasagne
b: foo bar baz
```

In the above snippet, the object `b` is initialized as a copy of `a`, and
further modifications to `a` and `b` will operate on disjoint underlying data.
Translated into Lumpy, the code above would take the form:

```
# test.lumpy
let a = ["foo", "bar", "baz"];
let b = a; # b becomes a *copy* of a
a.push("lasagne"); # modification of a does not affect b

println("a: " + repr(a));
println("b: " + repr(b));
```

```
$ lumpy test.lumpy
a: ["foo", "bar", "baz", "lasagne"]
b: ["foo", "bar", "baz"]
```

Unlike Lumpy, scripting languages such as JavaScript, Python, and Lua tend to
use reference semantics, with references to an object/table being assigned
during an assignment operation[^1].

JavaScript:

```
// test.js
a = ["foo", "bar", "baz"]
b = a; // a and b reference the same object
a.push("lasagne"); // modification of a affects b

console.log("a: " + a.join(", "))
console.log("b: " + b.join(", "))
```

```
$ node test.js
a: foo, bar, baz, lasagne
b: foo, bar, baz, lasagne
```

Python:

```
# test.py
a = ["foo", "bar", "baz"];
b = a; # a and b reference the same object
a.append("lasagne"); # modification of a affects b

print(f"a: {a}");
print(f"b: {b}");
```

```
$ python3 test.py
a: ['foo', 'bar', 'baz', 'lasagne']
b: ['foo', 'bar', 'baz', 'lasagne']
```

Lua:

```
# test.lua
a = {"foo", "bar", "baz"}
b = a; -- a and b reference the same object
a[#a+1] = "lasagne"; -- modification of a affects b

print("a: " .. table.concat(a, ", "))
print("b: " .. table.concat(b, ", "))
```

```
$ lua test.lua
a: foo, bar, baz, lasagne
b: foo, bar, baz, lasagne
```

Although I think that reference semantics are a reasonable default for a
programming language, I am personally not a huge reference semantics fan. In
languages with reference semantics, one usually has to do some sort of verbose
song and dance to actually copy the contents of one object to another. In Java
one needs to use
[`Object.clone`](https://docs.oracle.com/javase/8/docs/api/java/lang/Object.html#clone--).
In Python one needs to use [`copy.copy` or
`copy.deepcopy`](https://docs.python.org/3/library/copy.html). In JavaScript
one needs to use
[structuredClone](https://developer.mozilla.org/en-US/docs/Web/API/structuredClone).
And the list goes on... It all feels kind of hacky and inelegant.

One of the motivating reasons behind the creation of Lumpy was to see if there
was a way to make value semantics work in a scripting language without the
language or interpreter becoming too complicated. And as it turns out the
answer is "yes, actually". Work on Lumpy began at the start of August, and
within about two weeks the core language was pretty much finished. Then, it
only took me couple of weeks after that to put some polish on the language, add
in a useful set of builtin functions, flesh out a couple of example programs,
and build a small game development framework as a proof-of-concept library. In
the end, Lumpy ended up as a simple language with a coherent set of rules all
implemented with a relatively small amount of code (less than 5000 lines of
Python).

With the language more or less finished, there are a bunch of interesting areas
of study that are now open to exploration. Programming with value semantics
makes it incredibly easy to implement data structures and algorithms, so I
could absolutely see myself working through a book like [Open Data
Structures](https://opendatastructures.org/) or
[CLRS](https://en.wikipedia.org/wiki/Introduction_to_Algorithms) with Lumpy as
the implementation language. I haven't mentioned it at all, but Lumpy has most
of the builtin tools you would want when implementing abstract algorithms:
dynamic arrays, maps, sets, explicit references, operator overloading, metamaps
(similar to metatables in Lua), and a nice helping of syntax sugar to make
common operations easy to write.

Merge Sort in Lumpy:

```
# sort.lumpy - merge sort implemented in Lumpy

let sort = function(x) {
    if x.count() <= 1 {
        return x;
    }
    let mid = (x.count() / 2).trunc();
    let lo = sort(x.slice(0, mid));
    let hi = sort(x.slice(mid, x.count()));
    let lo_index = 0;
    let hi_index = 0;
    let result = [];
    for _ in x.count() {
        if lo_index == lo.count() {
            result.push(hi[hi_index]);
            hi_index = hi_index + 1;
        }
        elif hi_index == hi.count() {
            result.push(lo[lo_index]);
            lo_index = lo_index + 1;
        }
        elif lo[lo_index] < hi[hi_index] {
            result.push(lo[lo_index]);
            lo_index = lo_index + 1;
        }
        else {
            result.push(hi[hi_index]);
            hi_index = hi_index + 1;
        }
    }
    return result;
};

let vec = [6, 8, 3, 1, 7, 2, 4, 5, 9];
println("unsorded vector is " + repr(vec));
println("sorted vector is " + repr(sort(vec)));
```

```
$ lumpy sort.lumpy
unsorded vector is [6, 8, 3, 1, 7, 2, 4, 5, 9]
sorted vector is [1, 2, 3, 4, 5, 6, 7, 8, 9]
```

Depth-First-Search in Lumpy:

```
# dfs.lumpy - depth first search implemented in Lumpy

let node = {
    "new": function(name) {
        let self = {"name": name, "children": set{}};
        setmeta(self.&, node);
        return self;
    },
    "string": function(value) {
        return value.name;
    },
};

let graph = {
    "new": function(nodes) {
        let self = {"nodes": nodes};
        setmeta(self.&, graph);
        return self;
    },
};

let dfs = function(graph) {
    let visited = set{};
    let ordered = [];

    let visit = function(node) {
        if visited.contains(node) {
            return;
        }
        visited.insert(node);
        for child in node.*.children {
            visit(child);
        }
        ordered.insert(0, node.*);
    };

    for node in graph.nodes {
        visit(node);
    }
    return ordered;
};

# B-->E-->A--+
# |   |   |  |
# |   v   |  |
# +-->C<--+  |
#      |     v
#      +---->D

let a = node::new("A");
let b = node::new("B");
let c = node::new("C");
let d = node::new("D");
let e = node::new("E");

a.children.insert(c.&);
a.children.insert(d.&);

b.children.insert(c.&);
b.children.insert(e.&);

c.children.insert(d.&);

e.children.insert(a.&);
e.children.insert(c.&);

let ordered = dfs(graph::new({a.&, b.&, c.&, d.&, e.&}));
println("Ordered:");
for x in ordered {
    println(x);
}
```

```
$ lumpy dfs.lumpy
Ordered:
B
E
A
C
D
```

The game framework I mentioned earlier is a small wrapper around Pygame written
using a somewhat jank system that allows the interpreter to be extended at
runtime. Both the framework and the runtime extension stuff seem like they
would be worth exploring in more depth.

<video controls width="100%">
  <source src="/blog/2024-09-02-scripting-with-value-semantics-using-lumpy/lumpy-game.mp4">
</video>

The Lumpy interpreter also uses a relatively straightforward copy-on-write
system that makes collection types cheap to copy. The copy-on-write system is
not novel by any means, but there is definitely a blog post waiting to be
written about how copy-on-write is set up in the Lumpy interpreter. I could go
on, but the main point I am trying to get at here is that Lumpy is in a sweet
spot right now. There are a lot of areas to explore, but no hints of the same
sort of language developer burnout that I was feeling after working on
[Sunder](https://github.com/ashn-dot-dev/sunder) for like three years, and that
is something I am excited about! ☺️

## Footnotes
[^1]:
For good reason too! I am kind of glossing over what the definition of "value"
actually is. Lumpy defines a value as either primitive immutable data (such as
a boolean or number) or a collection of other values (ordered list of values in
the case of a vector, a group of key-value pairs with unique keys in the case
of a map, or a group of unique values in the case of a set). Lumpy considers a
value to be copied when that primitive data or all elements of that collection
are copied (from a user-visible perspective, Lumpy uses copy-on-write sharing
under the hood). But that definition gets a little tricky when you consider
that a file descriptor or database handle carries an implicit idea of a
"resource" that does not cleanly fit into the "copy all the data" model of
value semantics used by Lumpy. Part of the reason that languages such as C++
and Rust are so overwhelmingly complicated is because they have very specific
rules used to model and track the lifetimes of these resources. Scripting
languages tend to be more on the simple side, so reference semantics provide a
way to model these resources without having to introduce the idea of a copy
constructor into the language (one reference corresponds to one resource).
Lumpy sidesteps the problem altogether through the use of "external" types
which are basically anything not in the world of the "copy all the data" model.
