# test.lua
a = {"foo", "bar", "baz"}
b = a; -- a and b reference the same object
a[#a+1] = "lasagne"; -- modification of a affects b

print("a: " .. table.concat(a, ", "))
print("b: " .. table.concat(b, ", "))
