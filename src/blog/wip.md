Porting Code from C++ to Sunder
===============================

[Sunder](https://github.com/ashn-dot-dev/sunder) is a systems programming
language in the same language-family as C and C++. The process of porting code
from C or C++ to Sunder is usually straightforward, as the languages share many
of the same idioms and can express high-level concepts in similar ways. In this
blog post we will walk through the process of porting a merge-sort
implementation from C++ to Sunder.

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

would appear in Sunder as template function signature:

```
func mergeSort[[T]](a: []T) void
```

And because Sunder style favors snake_case over camelCase, we will choose to
write our function signature as `merge_sort` instead of `mergeSort`:

```
func merge_sort[[T]](a: []T) void
```

Alright so let's begin porting body of the ODS implementation to Sunder. The
first line of the ODS implementation is the recursive check that bails out when
there are zero or one elements to sort (i.e. the collection is already sorted):

```cpp
	if (a.length <= 1) return;
```

Slices in Sunder do not have a `length` member, but the `countof` operator
serves a similar function, returning the number of elements in a slice:

```
    if countof(a) <= 1 {
        return;
    }
```

The next four lines of the ODS implementation create two `ods:array`s, `a0`,
and `a1`, and populate those arrays with copies of the first and second halves
of the input array:

```cpp
	array<T> a0(0);
	array<T>::copyOfRange(a0, a, 0, a.length/2);
	array<T> a1(0);
	array<T>::copyOfRange(a1, a, a.length/2, a.length);
```

Sunder's standard library has
[`std::slice[[T]]::new`](https://github.com/ashn-dot-dev/sunder/blob/2022.10.01/lib/std/core.sunder#L102)
and
[`std::slice[[T]]::copy`](https://github.com/ashn-dot-dev/sunder/blob/2022.10.01/lib/std/core.sunder#L36)
which we can use to dynamically allocate and populate our own `a0` and `a1`
slices in the Sunder implementation. However, it is here that we encounter the
first instance in which we will need to diverge from the ODS implementation.
The `ods::array` container dynamically allocates memory in the class
constructor using the global `new[]` and frees memory with the global
`delete[]`. Actually, most C++ data structures and algorithms in the wild
manage memory using the global `new`/`delete` family of functions[^1]. But
Sunder has no concept of a global allocator, and idiomatic Sunder will require
the user to provide their own
[std::allocator](https://github.com/ashn-dot-dev/sunder/blob/2022.10.01/lib/std/core.sunder#L976)
instance to any function, data structure, or subsystem which requires dynamic
memory management.

Eventually we will want our `merge_sort` function to take an `allocator`
argument specifying which allocator should be used for temporary slice
allocation, but for the time being we will create a
[`std::general_allocator`](https://github.com/ashn-dot-dev/sunder/blob/2022.10.01/lib/std/general_allocator.sunder#L27)
instance within the `merge_sort` function itself:

```
    var allocator = std::general_allocator::init();
    var allocator = std::allocator::init[[std::general_allocator]](&allocator);
```

The first line:

```
    var allocator = std::general_allocator::init();
```

declares and initializes a `std::general_allocator` instance to keep track of
heap allocations. The second line:

```
    var allocator = std::allocator::init[[std::general_allocator]](&allocator);
```

takes the concrete `std::general_allocator` instance and creates an instance of
the `std::allocator` interface type used by functions that accept a
user-defined allocator. This second declaration of the `allocator` variable
intentionally shadows the first declaration of `allocator` variable since all
memory management within the `merge_sort` function should be working through
the abstract `std::allocator` interface independent of the underlying
allocator.

With an allocator in hand we can now allocate and populate our `a0` and `a1`
slices:

```
    var a0 = std::slice[[T]]::new(allocator, countof(a)/2);
    std::slice[[T]]::copy(a0, a[0:countof(a)/2]);
    var a1 = std::slice[[T]]::new(allocator, countof(a) - countof(a)/2);
    std::slice[[T]]::copy(a1, a[countof(a)/2:countof(a)]);
```

In the ODS implementation the `a0` and `a1` `ods::array` objects will
`delete[]` their allocated memory at scope exit when the `~ods::array`
destructor is run for each object. Sunder does not have destructors, but we can
achieve similar resource cleanup using the `defer` statement, which will
execute a block of code at scope exit. In this case, we will `defer` the
deallocation of our `a0` and `a1` array:

```
    defer {
        std::slice[[T]]::delete(allocator, a0);
        std::slice[[T]]::delete(allocator, a1);
    }
```

Finally, we add our recursive merge-sort calls and a call to merge the two
sub-slices:

```
    merge_sort[[T]](a0);
    merge_sort[[T]](a1);
    merge[[T]](a0, a1, a);
```

Of course we still need to implement the `merge` routine:
*TODO*

--------------------------------------------------------------------------------
TODO:
+ Implement `merge`
+ Show setup of a basic `main`
+ Improve code in subsequent versions

--------------------------------------------------------------------------------

Version 1:
```
# merge-sort-version-1.sunder
import "std";

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

func merge_sort[[T]](a: []T) void {
    if countof(a) <= 1 {
        return;
    }

    var allocator = std::general_allocator::init();
    var allocator = std::allocator::init[[std::general_allocator]](&allocator);

    var a0 = std::slice[[T]]::new(allocator, countof(a)/2);
    std::slice[[T]]::copy(a0, a[0:countof(a)/2]);
    var a1 = std::slice[[T]]::new(allocator, countof(a) - countof(a)/2);
    std::slice[[T]]::copy(a1, a[countof(a)/2:countof(a)]);
    defer {
        std::slice[[T]]::delete(allocator, a0);
        std::slice[[T]]::delete(allocator, a1);
    }


    merge_sort[[T]](a0);
    merge_sort[[T]](a1);
    merge[[T]](a0, a1, a);
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

Version 2:
```
# merge-sort-version-2.sunder
import "std";

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

func merge_sort[[T]](a: []T, allocator: std::allocator) void {
    if countof(a) <= 1 {
        return;
    }

    var a0 = std::slice[[T]]::new(allocator, countof(a)/2);
    var a1 = std::slice[[T]]::new(allocator, countof(a) - countof(a)/2);
    defer {
        std::slice[[T]]::delete(allocator, a0);
        std::slice[[T]]::delete(allocator, a1);
    }
    std::slice[[T]]::copy(a0, a[0:countof(a)/2]);
    std::slice[[T]]::copy(a1, a[countof(a)/2:countof(a)]);

    merge_sort[[T]](a0, allocator);
    merge_sort[[T]](a1, allocator);
    merge[[T]](a0, a1, a);
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
    var allocator = std::general_allocator::init();
    defer {
        allocator.fini();
    }
    var allocator = std::allocator::init[[std::general_allocator]](&allocator);

    var slice = (:[]ssize)[4, 8, 5, 3, 1, 6, 2, 7];
    display[[ssize]](slice);
    merge_sort[[ssize]](slice, allocator);
    display[[ssize]](slice);

    var slice = (:[][]byte)["banana", "carrot", "apple"];
    display[[[]byte]](slice);
    merge_sort[[[]byte]](slice, allocator);
    display[[[]byte]](slice);
}
```

Version 3:
```
# merge-sort-version-3.sunder
import "std";

func merge_sort[[T]](a: []T, allocator: std::allocator) void {
    if countof(a) <= 1 {
        return;
    }

    var a0 = std::slice[[T]]::new(allocator, countof(a)/2);
    var a1 = std::slice[[T]]::new(allocator, countof(a) - countof(a)/2);
    defer {
        std::slice[[T]]::delete(allocator, a0);
        std::slice[[T]]::delete(allocator, a1);
    }
    std::slice[[T]]::copy(a0, a[0:countof(a)/2]);
    std::slice[[T]]::copy(a1, a[countof(a)/2:countof(a)]);

    merge_sort[[T]](a0, allocator);
    merge_sort[[T]](a1, allocator);

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
    var allocator = std::general_allocator::init();
    defer {
        allocator.fini();
    }
    var allocator = std::allocator::init[[std::general_allocator]](&allocator);

    var slice = (:[]ssize)[4, 8, 5, 3, 1, 6, 2, 7];
    display[[ssize]](slice);
    merge_sort[[ssize]](slice, allocator);
    display[[ssize]](slice);

    var slice = (:[][]byte)["banana", "carrot", "apple"];
    display[[[]byte]](slice);
    merge_sort[[[]byte]](slice, allocator);
    display[[[]byte]](slice);
}
```

Version 4:
```
# merge-sort-version-4.sunder
import "std";

func merge_sort[[T]](slice: []T, allocator: std::allocator) void {
    if countof(slice) <= 1 {
        return;
    }

    var slice0 = std::slice[[T]]::new(allocator, countof(slice)/2);
    var slice1 = std::slice[[T]]::new(allocator, countof(slice) - countof(slice)/2);
    defer {
        std::slice[[T]]::delete(allocator, slice0);
        std::slice[[T]]::delete(allocator, slice1);
    }
    std::slice[[T]]::copy(slice0, slice[0:countof(slice)/2]);
    std::slice[[T]]::copy(slice1, slice[countof(slice)/2:countof(slice)]);

    merge_sort[[T]](slice0, allocator);
    merge_sort[[T]](slice1, allocator);

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
    var allocator = std::general_allocator::init();
    defer {
        allocator.fini();
    }
    var allocator = std::allocator::init[[std::general_allocator]](&allocator);

    var slice = (:[]ssize)[4, 8, 5, 3, 1, 6, 2, 7];
    display[[ssize]](slice);
    merge_sort[[ssize]](slice, allocator);
    display[[ssize]](slice);

    var slice = (:[][]byte)["banana", "carrot", "apple"];
    display[[[]byte]](slice);
    merge_sort[[[]byte]](slice, allocator);
    display[[[]byte]](slice);
}
```

--------------------------------------------------------------------------------

## Footnotes
[^1]:
I can count the number of custom C++ allocators I have seen in production code
on one hand.
