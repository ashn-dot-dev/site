# test.py
a = ["foo", "bar", "baz"];
b = a; # a and b reference the same object
a.append("lasagne"); # modification of a affects b

print(f"a: {a}");
print(f"b: {b}");
