// test.js
a = ["foo", "bar", "baz"]
b = a; // a and b reference the same object
a.push("lasagne"); // modification of a affects b

console.log("a: " + a.join(", "))
console.log("b: " + b.join(", "))
