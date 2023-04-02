Porting Some Code from C++ to Sunder
====================================

[Sunder](https://github.com/ashn-dot-dev/sunder) is a systems programming
language in the same language-family as C and C++. The process of porting code
from C or C++ to Sunder is usually straightforward, as the languages share many
of the same idioms and can express high-level concepts in similar ways. In this
blog post we will walk through the process of porting a merge-sort
implementation from C++ to Sunder[^1].

Our initial merge-sort code comes from the C++ version of Open Data Structures
(Edition 0.1G), chapter 11.1 - Comparison-Based Sorting. The code can be found
in the online textbook
[here](https://opendatastructures.org/ods-cpp/11_1_Comparison_Based_Sorti.html)
and in the Open Data Structures GitHub repository
[here](https://github.com/patmorin/ods/blob/edition-0.1g/cpp/Algorithms.h#L83).

```cpp
template<class T>
void merge(array<T> &a0, array<T> &a1, array<T> &a) {
	int i0 = 0, i1 = 0;
	for (int i = 0; i < a.length; i++) {
		if (i0 == a0.length)
			a[i] = a1[i1++];
		else if (i1 == a1.length)
			a[i] = a0[i0++];
		else if (compare(a0[i0], a1[i1]) < 0)
			a[i] = a0[i0++];
		else
			a[i] = a1[i1++];
	}
}

template<class T>
void mergeSort(array<T> &a) {
	if (a.length <= 1) return;
	array<T> a0(0);
	array<T>::copyOfRange(a0, a, 0, a.length/2);
	array<T> a1(0);
	array<T>::copyOfRange(a1, a, a.length/2, a.length);
	mergeSort(a0);
	mergeSort(a1);
	merge(a0, a1, a);
}
```

The Open Data Structures (ODS) implementation operates on an
[`ods::array`](https://github.com/patmorin/ods/blob/edition-0.1g/cpp/array.h)
of some type `T`. The closest equivalent to `ods::array` would be Sunder's
slice type, so our merge-sort implementation will operate on a slice of
some type `T`. The generic C++ template function signature:

```cpp
template<class T>
void mergeSort(array<T> &a)
```

would appear in Sunder as the template function signature:

```
func mergeSort[[T]](a: []T) void
```

And because Sunder style favors snake_case over camelCase, we will choose to
write our function name as `merge_sort` instead of `mergeSort`:

```
func merge_sort[[T]](a: []T) void
```

Alright so let's begin porting the body of the ODS implementation to Sunder.
The first line of the ODS implementation is a recursive check that bails out
when there are zero or one elements to sort (i.e. the collection is already
sorted):

```cpp
if (a.length <= 1) return;
```

Slices in Sunder do not have a `length` member, but the `countof` operator
serves a similar purpose, returning the number of elements in a slice:

```
if countof(a) <= 1 {
    return;
}
```

The next four lines of the ODS implementation create two `ods:array`s, `a0`,
and `a1`, and populate those arrays with copies of the first and second halves
of the input array using `ods::array<T>::copyOfRange`:

```cpp
array<T> a0(0);
array<T>::copyOfRange(a0, a, 0, a.length/2);
array<T> a1(0);
array<T>::copyOfRange(a1, a, a.length/2, a.length);
```

Sunder's standard library has
[`std::slice[[T]]::new`](https://github.com/ashn-dot-dev/sunder/blob/2023.03.31/lib/std/std.sunder#L903)
and
[`std::slice[[T]]::copy`](https://github.com/ashn-dot-dev/sunder/blob/2023.03.31/lib/std/std.sunder#L820)
which we can use to dynamically allocate and populate our own `a0` and `a1`
slices in the Sunder implementation.

```
var a0 = std::slice[[T]]::new(countof(a)/2);
var a1 = std::slice[[T]]::new(countof(a) - countof(a)/2);
defer {
    std::slice[[T]]::delete(a0);
    std::slice[[T]]::delete(a1);
}
std::slice[[T]]::copy(a0, a[0:countof(a)/2]);
std::slice[[T]]::copy(a1, a[countof(a)/2:countof(a)]);
```

In the ODS implementation the `a0` and `a1` objects will automatically
`delete[]` allocated memory at scope exit when the `ods::array` destructor is
run for each object. Sunder does not have destructors, but we can achieve
similar resource cleanup using a `defer` statement to deallocate the `a0` and
`a1` arrays with
[`std::slice[[T]]::delete`](https://github.com/ashn-dot-dev/sunder/blob/2023.03.31/lib/std/std.sunder#L944)
at scope exit:

```
var a0 = std::slice[[T]]::new(countof(a)/2);
var a1 = std::slice[[T]]::new(countof(a) - countof(a)/2);
defer {
    std::slice[[T]]::delete(a0);
    std::slice[[T]]::delete(a1);
}
std::slice[[T]]::copy(a0, a[0:countof(a)/2]);
std::slice[[T]]::copy(a1, a[countof(a)/2:countof(a)]);
```

Finally, we add our recursive merge-sort calls as well as a call to merge the
two sub-slices:

```
merge_sort[[T]](a0);
merge_sort[[T]](a1);
merge[[T]](a0, a1, a);
```

All together the Sunder `merge_sort` function takes the form:

```
func merge_sort[[T]](a: []T) void {
    if countof(a) <= 1 {
        return;
    }

    var a0 = std::slice[[T]]::new(countof(a)/2);
    var a1 = std::slice[[T]]::new(countof(a) - countof(a)/2);
    defer {
        std::slice[[T]]::delete(a0);
        std::slice[[T]]::delete(a1);
    }
    std::slice[[T]]::copy(a0, a[0:countof(a)/2]);
    std::slice[[T]]::copy(a1, a[countof(a)/2:countof(a)]);

    merge_sort[[T]](a0);
    merge_sort[[T]](a1);
    merge[[T]](a0, a1, a);
}
```

Of course we still need to implement the `merge` function. As a refresher, the
ODS implementation of `merge` is:

```cpp
template<class T>
void merge(array<T> &a0, array<T> &a1, array<T> &a) {
int i0 = 0, i1 = 0;
	for (int i = 0; i < a.length; i++) {
		if (i0 == a0.length)
			a[i] = a1[i1++];
		else if (i1 == a1.length)
			a[i] = a0[i0++];
		else if (compare(a0[i0], a1[i1]) < 0)
			a[i] = a0[i0++];
		else
			a[i] = a1[i1++];
	}
}
```

The generic C++ template function signature:

```cpp
template<class T>
void merge(array<T> &a0, array<T> &a1, array<T> &a)
```

would appear in Sunder as the template function signature:

```
func merge[[T]](a0: []T, a1: []T, a: []T) void
```

The declarations of `i0` and `i1` in the first line of the ODS implementation:

```cpp
int i0 = 0, i1 = 0;
```

become two separate declaration statements in the Sunder implementation:

```
var i0 = 0u;
var i1 = 0u;
```

Sunder does not have a traditional C-style for-loop, but the common form of a
C-style for-loop, `for (int i = 0; i < end, i++)` can be expressed using the
Sunder for-loop syntax `for i in end`. So the ODS loop:

```cpp
for (int i = 0; i < a.length; i++) {
	// loop body...
}
```

translates to Sunder as:

```
for i in countof(a) {
    # loop body...
}
```

with us once again using the `countof` operator to retrieve the length of `a`.

The body of the loop contains an if-else chain that translates to Sunder quite
easily:

```
if i0 == countof(a0) {
    a[i] = a1[i1];
    i1 = i1 + 1;
}
elif i1 == countof(a1) {
    a[i] = a0[i0];
    i0 = i0 + 1;
}
elif std::compare[[T]](&a0[i0], &a1[i1]) < 0 {
    a[i] = a0[i0];
    i0 = i0 + 1;
}
else {
    a[i] = a1[i1];
    i1 = i1 + 1;
}
```

The assign-and-increment-index statement in C++:

```cpp
a[i] = a1[i1++];
```

translates as two separate statements, as prefix and postfix increment
operators are explicitly not supported in Sunder:

```
a[i] = a1[i1];
i1 = i1 + 1;
```

The [`ods::compare`](https://github.com/patmorin/ods/blob/edition-0.1g/cpp/utils.h#L23)
function used to compare elements of the `a0` and `a1` arrays behaves similar
to C's `strcmp` function or Java's `compareTo` method, returning a signed
integer less than, equal to, or greater than zero if the first operand is less
than, equal to, or greater than the second operand, respectively. Sunder has a
function,
[`std::compare`](https://github.com/ashn-dot-dev/sunder/blob/2023.03.31/lib/std/std.sunder#L1788),
which performs this same comparison for two comparable objects of type `T`[^2].

All together the Sunder `merge` function takes the form:

```
func merge[[T]](a0: []T, a1: []T, a: []T) void {
    var i0 = 0u;
    var i1 = 0u;

    for i in countof(a) {
        if i0 == countof(a0) {
            a[i] = a1[i1];
            i1 = i1 + 1;
        }
        elif i1 == countof(a1) {
            a[i] = a0[i0];
            i0 = i0 + 1;
        }
        elif std::compare[[T]](&a0[i0], &a1[i1]) < 0 {
            a[i] = a0[i0];
            i0 = i0 + 1;
        }
        else {
            a[i] = a1[i1];
            i1 = i1 + 1;
        }
    }
}
```

The initial versions of our `merge_sort` and `merge` functions are complete.
Before we do anything else, it would probably be a good idea to test our
sorting algorithm to make sure nothing was missed in translation. Sunder has a
standard testing program, `sunder-test`, that ships as part of the Sunder
toolchain. However, I want this blog post to focus purely on porting code, so
for now we will settle for ad hoc print-and-eyeball-the-output testing.

Using a hastily thrown together `display` function:

```
func display[[T]](slice: []T) void {
    std::print(std::out(), "[");
    for i in countof(slice) {
        if i != 0 {
            std::print(std::out(), ", ");
        }
        std::print_format(std::out(), "{}", (:[]std::formatter)[std::formatter::init[[T]](&slice[i])]);
    }
    std::print_line(std::out(), "]");
}
```

we can verify that `merge_sort` correctly sorts a slice by `display`ing the
elements of an initially unsorted slice before and after `merge_sort` is
called:

```
func main() void {
    var slice = (:[]ssize)[4, 8, 5, 3, 1, 6, 2, 7];
    display[[ssize]](slice);
    merge_sort[[ssize]](slice);
    display[[ssize]](slice);

    var slice = (:[][]byte)["banana", "carrot", "apple"];
    display[[[]byte]](slice);
    merge_sort[[[]byte]](slice);
    display[[[]byte]](slice);
}
```

All together this is version 1 of our merge sort implementation and ad hoc
tests:

```
# merge-sort-version-1.sunder
import "std";

func merge_sort[[T]](a: []T) void {
    if countof(a) <= 1 {
        return;
    }

    var a0 = std::slice[[T]]::new(countof(a)/2);
    var a1 = std::slice[[T]]::new(countof(a) - countof(a)/2);
    defer {
        std::slice[[T]]::delete(a0);
        std::slice[[T]]::delete(a1);
    }
    std::slice[[T]]::copy(a0, a[0:countof(a)/2]);
    std::slice[[T]]::copy(a1, a[countof(a)/2:countof(a)]);

    merge_sort[[T]](a0);
    merge_sort[[T]](a1);
    merge[[T]](a0, a1, a);
}

func merge[[T]](a0: []T, a1: []T, a: []T) void {
    var i0 = 0u;
    var i1 = 0u;

    for i in countof(a) {
        if i0 == countof(a0) {
            a[i] = a1[i1];
            i1 = i1 + 1;
        }
        elif i1 == countof(a1) {
            a[i] = a0[i0];
            i0 = i0 + 1;
        }
        elif std::compare[[T]](&a0[i0], &a1[i1]) < 0 {
            a[i] = a0[i0];
            i0 = i0 + 1;
        }
        else {
            a[i] = a1[i1];
            i1 = i1 + 1;
        }
    }
}

func display[[T]](slice: []T) void {
    std::print(std::out(), "[");
    for i in countof(slice) {
        if i != 0 {
            std::print(std::out(), ", ");
        }
        std::print_format(std::out(), "{}", (:[]std::formatter)[std::formatter::init[[T]](&slice[i])]);
    }
    std::print_line(std::out(), "]");
}

func main() void {
    var slice = (:[]ssize)[4, 8, 5, 3, 1, 6, 2, 7];
    display[[ssize]](slice);
    merge_sort[[ssize]](slice);
    display[[ssize]](slice);

    var slice = (:[][]byte)["banana", "carrot", "apple"];
    display[[[]byte]](slice);
    merge_sort[[[]byte]](slice);
    display[[[]byte]](slice);
}
```

```sh
$ sunder-run merge-sort-version-1.sunder
[4, 8, 5, 3, 1, 6, 2, 7]
[1, 2, 3, 4, 5, 6, 7, 8]
[banana, carrot, apple]
[apple, banana, carrot]
```

Horray, that output certainly looks sorted to me!

Okay so we could declare our port mission-accomplished here, but I think there
is still a bit of cleanup that we could do if we want to make our ported code
look like idiomatic Sunder. Currently we have two functions, `merge_sort` and
`merge`, but `merge` is only ever called from within `merge_sort`. Sunder style
generally prefers longer functions where all the logic is in one place rather
than multiple smaller functions where logic is spread out. The easy solution in
our case is to inline the contents of `merge` into our `merge_sort` function:

```
# merge-sort-version-2.sunder
import "std";

func merge_sort[[T]](a: []T) void {
    if countof(a) <= 1 {
        return;
    }

    var a0 = std::slice[[T]]::new(countof(a)/2);
    var a1 = std::slice[[T]]::new(countof(a) - countof(a)/2);
    defer {
        std::slice[[T]]::delete(a0);
        std::slice[[T]]::delete(a1);
    }
    std::slice[[T]]::copy(a0, a[0:countof(a)/2]);
    std::slice[[T]]::copy(a1, a[countof(a)/2:countof(a)]);

    merge_sort[[T]](a0);
    merge_sort[[T]](a1);

    var i0 = 0u;
    var i1 = 0u;
    for i in countof(a) {
        if i0 == countof(a0) {
            a[i] = a1[i1];
            i1 = i1 + 1;
        }
        elif i1 == countof(a1) {
            a[i] = a0[i0];
            i0 = i0 + 1;
        }
        elif std::compare[[T]](&a0[i0], &a1[i1]) < 0 {
            a[i] = a0[i0];
            i0 = i0 + 1;
        }
        else {
            a[i] = a1[i1];
            i1 = i1 + 1;
        }
    }
}

func display[[T]](slice: []T) void {
    std::print(std::out(), "[");
    for i in countof(slice) {
        if i != 0 {
            std::print(std::out(), ", ");
        }
        std::print_format(std::out(), "{}", (:[]std::formatter)[std::formatter::init[[T]](&slice[i])]);
    }
    std::print_line(std::out(), "]");
}

func main() void {
    var slice = (:[]ssize)[4, 8, 5, 3, 1, 6, 2, 7];
    display[[ssize]](slice);
    merge_sort[[ssize]](slice);
    display[[ssize]](slice);

    var slice = (:[][]byte)["banana", "carrot", "apple"];
    display[[[]byte]](slice);
    merge_sort[[[]byte]](slice);
    display[[[]byte]](slice);
}
```

Running our ad hoc tests again, we can see that this version 2 of our merge
sort implementation behaves identically to our version 1 implementation:

```
$ sunder-run merge-sort-version-2.sunder
[4, 8, 5, 3, 1, 6, 2, 7]
[1, 2, 3, 4, 5, 6, 7, 8]
[banana, carrot, apple]
[apple, banana, carrot]
```

Finally, it would be good to take a pass over the `merge_sort` function and see
about using more idiomatic names for parameters and local variables. The ODS
implementation uses names like `a`, `a0`, and `a1`, because the ODS
implementation (presumably) sorts containers of type `ods::array`. But our
Sunder implementation sorts slices, so more appropriate names for these
parameters would be `slice`, `slice0`, and `slice1`, following the convention
used by the standard library for functions that perform operations on slice
types:

```
# merge-sort-version-3.sunder
import "std";

func merge_sort[[T]](slice: []T) void {
    if countof(slice) <= 1 {
        return;
    }

    var slice0 = std::slice[[T]]::new(countof(slice)/2);
    var slice1 = std::slice[[T]]::new(countof(slice) - countof(slice)/2);
    defer {
        std::slice[[T]]::delete(slice0);
        std::slice[[T]]::delete(slice1);
    }
    std::slice[[T]]::copy(slice0, slice[0:countof(slice)/2]);
    std::slice[[T]]::copy(slice1, slice[countof(slice)/2:countof(slice)]);

    merge_sort[[T]](slice0);
    merge_sort[[T]](slice1);

    var i0 = 0u;
    var i1 = 0u;
    for i in countof(slice) {
        if i0 == countof(slice0) {
            slice[i] = slice1[i1];
            i1 = i1 + 1;
        }
        elif i1 == countof(slice1) {
            slice[i] = slice0[i0];
            i0 = i0 + 1;
        }
        elif std::compare[[T]](&slice0[i0], &slice1[i1]) < 0 {
            slice[i] = slice0[i0];
            i0 = i0 + 1;
        }
        else {
            slice[i] = slice1[i1];
            i1 = i1 + 1;
        }
    }
}

func display[[T]](slice: []T) void {
    std::print(std::out(), "[");
    for i in countof(slice) {
        if i != 0 {
            std::print(std::out(), ", ");
        }
        std::print_format(std::out(), "{}", (:[]std::formatter)[std::formatter::init[[T]](&slice[i])]);
    }
    std::print_line(std::out(), "]");
}

func main() void {
    var slice = (:[]ssize)[4, 8, 5, 3, 1, 6, 2, 7];
    display[[ssize]](slice);
    merge_sort[[ssize]](slice);
    display[[ssize]](slice);

    var slice = (:[][]byte)["banana", "carrot", "apple"];
    display[[[]byte]](slice);
    merge_sort[[[]byte]](slice);
    display[[[]byte]](slice);
}
```

Unsurprisingly, running our ad hoc tests shows that version 3 of our merge sort
implementation behaves identically to our version 1 and version 2
implementations:

```
$ sunder-run merge-sort-version-3.sunder
[4, 8, 5, 3, 1, 6, 2, 7]
[1, 2, 3, 4, 5, 6, 7, 8]
[banana, carrot, apple]
[apple, banana, carrot]
```

With that last bit of cleanup done, I think we can call our C++-to-Sunder
merge-sort port complete! For a relatively small amount of effort, we were able
to take existing C++ code and bring it to Sunder with few conceptual changes.
Of course, not all C and C++ code would be this easy to implement in Sunder,
but a surprising number of data structures and algorithms can be ported from C
or C++ to Sunder line-for-line. In fact, the process of porting merge-sort seen
in this blog post is almost exactly how Sunder's
[`std::sort`](https://github.com/ashn-dot-dev/sunder/blob/2023.03.31/lib/std/std.sunder#L1846)
was introduced into the Sunder standard library. Although this blog post was
more of an example porting journey rather than a porting guide, I hope it is a
little easier to see how Sunder compares to existing systems programming
languages, and how writing/porting software to Sunder might look like
from a development perspective.

--------------------------------------------------------------------------------

## Footnotes
[^1]:
Specifically [Sunder version 2023.03.31](https://github.com/ashn-dot-dev/sunder/releases/tag/2023.03.31).

[^2]:
Any type `T` that implements a `compare` member function with the signature
`func compare(lhs: *T, rhs: *T) ssize` is considered comparable. Comparable
types in the Sunder standard library include built-ins such as
[s32](https://github.com/ashn-dot-dev/sunder/blob/2023.03.31/lib/std/std.sunder#L4430)
and
[\[\]byte](https://github.com/ashn-dot-dev/sunder/blob/2023.03.31/lib/std/std.sunder#L4624),
as well as higher level manged types such as
[std::big_integer](https://github.com/ashn-dot-dev/sunder/blob/2023.03.31/lib/std/std.sunder#L2102)
and
[std::string](https://github.com/ashn-dot-dev/sunder/blob/2023.03.31/lib/std/std.sunder#L2800).
