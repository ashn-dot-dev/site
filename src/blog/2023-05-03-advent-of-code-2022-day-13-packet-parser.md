Implementing the Advent of Code 2022 Day 13 Packet Parser
=========================================================

In this post we are going to run through a lightning fast introduction to
parsing by writing a parser for the
[Advent of Code 2022 day 13](https://adventofcode.com/2022/day/13)
signal packet data using Python. The day 13 problem describes the signal packet
data as follows:

> Packet data consists of lists and integers. Each list starts with `[`, ends
> with `]`, and contains zero or more comma-separated values (either integers
> or other lists). Each packet is always a list and appears on its own line.

With some example packet pairs provided in the sample test input:

```
[1,1,3,1,1]
[1,1,5,1,1]

[[1],[2,3,4]]
[[1],4]

[9]
[[8,7,6]]

[[4,4],4,4]
[[4,4],4,4,4]

[7,7,7,7]
[7,7,7]

[]
[3]

[[[]]]
[[]]

[1,[2,[3,[4,[5,6,7]]]],8,9]
[1,[2,[3,[4,[5,6,0]]]],8,9]
```

The parser that we are going to write will parse one of these packets (i.e. one
line), and return the parsed packet as a Python `list` containing nested `list`
and `int` values. So parsing the input string
`"[1,2,3,[4,5],[[6]],[7,[8,9]]]"`, should produce a Python list that would
print as `[1, 2, 3, [4, 5], [[6]], [7, [8, 9]]]`.

## Quick Parsing Overview

A **parser** is a tool which transforms some input text into an equivalent
abstract data structure according to a set of rules known as a **grammar**. A
grammar may be described informally, as seen by the day 13 description of a
packet, or formally using some sort of grammar notation:

```
PACKET  = LIST
VALUE   = LIST | INTEGER
LIST    = "[" (VALUE ("," VALUE)*)? "]"
INTEGER = [1-9]+
```

Parsing usually consists of two phases:

1. Transforming the input text into a stream of tokens.
2. Building data structures from the stream of tokens according to rules based
   the next token(s) in the stream.

These two phases are usually called the **lexing** and **parsing** phases[^1].
In this context, lexing refers to the phase where a stream of tokens is created
from text, and parsing refers to the phase where data structures are built from
the token stream according to the rules of a grammar.

A **token** is an atomic unit of the language we are parsing. In English,
tokens are the individual words and punctuation that make up a sentence. The
English sentence `"Alice walked to the store."` contains the tokens:

```
Alice
walked
to
the
store
.
```

In the case of English, each word is one token, because the sentence would lose
meaning if you broke the sentence down into smaller components[^2]. In a
programming language such as Python, tokens tend to be things like keywords,
identifiers, punctuation, and literals such as integers and strings. The Python
expression `print("hello, world", 123)` contains the tokens:

```
print
(
"hello, world"
,
123
)
```

The day 13 packet language consists of the punctuation tokens `,`, `[`, and `]`
as well as integer tokens. The packet `[123,456,[789]]` contains the tokens:

```
[
123
,
456
,
[
789
]
]
```

Once input text is lexed into individual tokens by a lexer, a parser will
attempt to build data structures from those tokens based on the rules of a
grammar. English grammar is incredibly complicated[^3] and full of exceptions,
but we (generally) know a sentence is valid if our brains can interpret the
sentence (i.e. parse the sentence) as something that makes sense. In the
English sentence `"Alice walked to the store."` we can identify `Alice` as the
subject, `walk (past tense)` as the action associated with the subject, `to`
indicates that the next noun in the sentence was Alice's destination, `the` is
a determiner relating to the destination noun, and `store` is the destination
noun we expected. The period indicates that the sentence has concluded.

We can construct a parser that will perform the same kind of analysis for our
packet grammar, reading in tokens one at a time and seeing if the order of
those tokens makes sense for a packet. We will first start by writing a lexer.
Then, we will use that lexer to write a parser. Finally, we will wrap our lexer
and parser into one function that will accept a packet input string and print
the list parsed from that input string.

## Writing the Lexer

Our lexer is going to take some input as a string and then spit out tokens one
at a time whenever we call the `next_token` method on the lexer. Speaking of
tokens, they are pretty important, so we should probably define some sort of
`Token` class for our lexer and parser to use:

```python
from enum import Enum


class TokenKind(Enum):
    EOI = "end-of-input"
    COMMA = ","
    INTEGER = "integer"
    LBRACKET = "["
    RBRACKET = "]"


class Token:
    def __init__(self, kind, text):
        self.kind = kind
        self.text = text

    def __str__(self):
        if self.kind == TokenKind.INTEGER:
            return f"{self.kind.value}({self.text})"
        return f"{self.kind.value}"
```

Here we have a class called `Token` which contains two fields: `kind` and
`text`. The `kind` field of `Token` will be one of the values of the
`TokenKind` enum, indicating what sort of token we have. Earlier we identified
that the packet grammar consisted of the punctuation tokens `,`, `[`, and `]`
represented by `TokenKind.COMMA`, `TokenKind.LBRACKET`, `TokenKind.RBRACKET`,
as well as integer tokens represented by `TokenKind.INTEGER`. Observant readers
will notice the `TokenKind.EOI` token kind not previously mentioned. This is a
special token that indicates that the end of the input string has been reached
and no more tokens can be lexed. It is useful to have this special kind of
token so that the parser has a way to know that the entire input string has
been consumed. The `text` field of `Token` will be the slice of characters from
the input associated with the token. For the comma, left square bracket, and
right square bracket tokens, the `kind` and `text` fields will end up being
identical strings. For integer tokens, the `kind` field will indicate that the
token is an integer token, and the `text` field will contain the digits of the
parsed integer. The end-of-input token is typically represented using the empty
string.

```python
print(Token(TokenKind.COMMA, ","))
print(Token(TokenKind.LBRACKET, "["))
print(Token(TokenKind.RBRACKET, "]"))
print(Token(TokenKind.INTEGER, "123"))
print(Token(TokenKind.EOI, ""))
```

```sh
$ python3 parser.py
,
[
]
integer(123)
end-of-input
```

Our `Lexer` class will need two pieces of information to operate: the input
string and an integer keeping track of the current lexing position within the
input string. Since lexing always begins at the start of the input string, our
lexer only needs to accept the input string (called `source` in the `Lexer`
class) as an argument to the lexer's `__init__` function:

```
class Lexer:
    def __init__(self, source):
        self.source = source
        self.position = 0

lexer = Lexer("[1,2,3,[4,5],[[6]],[7,[8,9]]]")
print(f"source=\"{lexer.source}\", position={lexer.position}")
```

```sh
$ python3 parser.py
source="[1,2,3,[4,5],[[6]],[7,[8,9]]]", position=0
```

Next I want to create two helper methods: `is_eoi` and `current_character`. The
`is_eoi` method will return `True` if the lexer's position is past the end of
the input string (i.e. we have reached the end-of-input) and `False` otherwise:

```python
class Lexer:
    def __init__(self, source):
        self.source = source
        self.position = 0

    def is_eoi(self):
        return self.position >= len(self.source)
```

The `current_character` method will return the character of the input string
corresponding to the lexer's current position. Crucially, if the lexer has
reached the end-of-input then attempting to index the input string will produce
an `IndexError`, so we use `is_eoi` to check if the lexer has reached the
end-of-input and return the empty string in that case:

```python
class Lexer:
    EOI = ""

    def __init__(self, source):
        self.source = source
        self.position = 0

    def is_eoi(self):
        return self.position >= len(self.source)

    def current_character(self):
        if self.is_eoi():
            return Lexer.EOI
        return self.source[self.position]
```

Believe it or not, with just these two helper methods we have everything we
need to write the `next_token` method. When lexing tokens there are five cases
to consider:

1. The lexer has reached the end-of-input → return an `TokenKind.EOI` token
2. The current character is `,` → return a `TokenKind.COMMA` token
3. The current character is `[` → return a `TokenKind.LBRACKET` token
4. The current character is `]` → return a `TokenKind.RBRACKET` token
5. The current character is a digit → advance the position of the lexer until
   the current character is no longer a digit and return a `TokenKind.INTEGER`
   containing the digits as the token text

Our `next_token` method will essentially check for each of these cases using
an `if` statement. If the current character does not match any of our cases
then we will raise an exception to indicate that a lexing error has occurred:

```python
class Lexer:
    EOI = ""

    def __init__(self, source):
        self.source = source
        self.position = 0

    def is_eoi(self):
        return self.position >= len(self.source)

    def current_character(self):
        if self.is_eoi():
            return Lexer.EOI
        return self.source[self.position]

    def next_token(self):
        start = self.position

        if self.is_eoi():
            return Token(TokenKind.EOI, Lexer.EOI)

        if self.current_character() == TokenKind.COMMA.value:
            self.position += len(TokenKind.COMMA.value)
            return Token(TokenKind.COMMA, self.source[start : self.position])

        if self.current_character() == TokenKind.LBRACKET.value:
            self.position += len(TokenKind.LBRACKET.value)
            return Token(TokenKind.LBRACKET, self.source[start : self.position])

        if self.current_character() == TokenKind.RBRACKET.value:
            self.position += len(TokenKind.RBRACKET.value)
            return Token(TokenKind.RBRACKET, self.source[start : self.position])

        if self.current_character().isdigit():
            while self.current_character().isdigit():
                self.position += 1
            return Token(TokenKind.INTEGER, self.source[start : self.position])

        raise Exception(f"invalid character `{self.current_character()}`")
```

Testing our lexer on some example input, we can see that the lexer correctly
produces a valid stream of tokens for valid input and will raise an exception
for invalid input:

```python
print("VALID INPUT")
print("===========")
lexer = Lexer("[123,456,[789]]")
while (token := lexer.next_token()).kind != TokenKind.EOI:
    print(token)

print("\n", end="")

print("INVALID INPUT")
print("=============")
try:
    lexer = Lexer("[123,456,[$]]")
    while (token := lexer.next_token()).kind != TokenKind.EOI:
        print(token)
except Exception as e:
    print(e)
```

```sh
$ python3 parser.py
VALID INPUT
===========
[
integer(123)
,
integer(456)
,
[
integer(789)
]
]

INVALID INPUT
=============
[
integer(123)
,
integer(456)
,
[
invalid character `$`
```

With the addition of the `next_token` method, our lexer is complete. Not bad
for less than fifty lines of code!

## Writing the Parser

Our parser is going to take a lexer and use the lexer's `next_token` method to
parse a packet according to the rules of our packet grammar. The type of parser
we will be implementing today is what is known as a
[recursive descent parser](https://en.wikipedia.org/wiki/Recursive_descent_parser).
We will take our list of rules and implement a method for each rule. Based on
the day 13 description of the packet data, it appears grammar consists of four
rules:

1. A packet is a list followed by the end-of-input token
3. A value is either a list or an integer
2. A list starts with `[` token, ends with `]` token and contains zero or more
   values separated by `,` tokens
4. An integer consists of a single integer token

In each of our rules, we only ever need to look one token ahead to know what to
parse next. When parsing an integer we need to check one token ahead to make
sure that token is an integer. When parsing a list we need to check one token
ahead as the beginning of the rule to make sure the first token is a `[`, we
need to check one token ahead when parsing the values of the list in order to
see if we have reached the end of the list as indicated by a `]` token or if
there are still more values to parse as indicated by a `,` token. We need to
check one token ahead when parsing a value to know if we need to parse a list
as indicated by a `[` token or parse an integer as indicated by an integer
token. And finally we need to check one token ahead when parsing a packet in
order make sure we have reached the end-of-input after parsing the
list part of the packet.

Our `Parser` class will need two pieces of information to operate: the lexer
instance and the current token in the token stream (a.k.a the token one ahead
from all the tokens that we have already consumed). Since parsing always starts
with the first token produced by the lexer, our parser only needs to accept the
lexer instance (called `lexer` in the `Parser` class) as an argument to the
parser's `__init__` function. The current token in the token stream (called
`current` in the `Parser` class) is initialized in the parser's `__init__`
function:

```python
class Parser:
    def __init__(self, lexer):
        self.lexer = lexer
        self.next = lexer.next_token()
```

Next we will create a method for each of our four rules. For now each method
will raise a TODO exception, but we will replace each of those TODOs with a
parsing rule as we flesh out the parser:

```python
class Parser:
    def __init__(self, lexer):
        self.lexer = lexer
        self.next = lexer.next_token()

    def parse_packet(self):
        raise Exception("TODO")

    def parse_value(self):
        raise Exception("TODO")

    def parse_list(self):
        raise Exception("TODO")

    def parse_integer(self):
        raise Exception("TODO")
```

Parsing an integer is the easiest of the rules to implement. An integer
consists of a single integer token, so all we need to do is check to see if the
next token is of kind `TokenKind.INTEGER`:

```python
    def parse_integer(self):
        if self.current.kind != TokenKind.INTEGER:
            raise Exception(f"expected {TokenKind.INTEGER.value} (found {self.current})")
        result = int(self.current.text)
        self.current = self.lexer.next_token()
        return result
```

If the current token is not an integer then we raise an exception to indicate
that we did not encounter the expected token. Otherwise we obtain the value of
the integer as a python `int` with `result = int(self.current.text)` and
advance past the integer token with `self.current = self.lexer.next_token()`
before returning the parsed integer. Testing the `parse_integer` method with
some example input shows that our parser will return the expected integer when
an integer is encountered, and produce an error when a non-integer token is
encountered:

```python
lexer = Lexer("123")
parser = Parser(lexer)
print(parser.parse_integer())

try:
    lexer = Lexer("[")
    parser = Parser(lexer)
    print(parser.parse_integer())
except Exception as e:
    print(e)
```

```sh
$ python3 parser.py
123
expected integer (found `[`)
```

The `parse_list` method is a bit trickier. For the opening `[` and closing `]`
we can use the same pattern seen in the `parse_integer` method where we check
the current token and raise an exception if the current token is not a token we
were expecting:

```python
    def parse_list(self):
        if self.current.kind != TokenKind.LBRACKET:
            raise Exception(f"expected `{TokenKind.LBRACKET.value}` (found `{self.current}`)")
        self.current = self.lexer.next_token()

        result = []
        # TODO: Parse the comma-separated values...

        if self.current.kind != TokenKind.RBRACKET:
            raise Exception(f"expected `{TokenKind.RBRACKET.value}` (found `{self.current}`)")
        self.current = self.lexer.next_token()
        return result
```

But how do we parse the list of zero or more comma-separated values? We should
try to parse either a list or a integer depending on the current token. We also
need to make sure that we parse the comma separating the values, but only if at
least one value has already been parsed. The trick to parse zero or more of
something is to use a `while` loop that will only stop once the ending token
(in this case a `]`) becomes the current token in the parser:


```python
    def parse_list(self):
        if self.current.kind != TokenKind.LBRACKET:
            raise Exception(f"expected `{TokenKind.LBRACKET.value}` (found `{self.current}`)")
        self.current = self.lexer.next_token()

        result = []
        while self.current.kind != TokenKind.RBRACKET:
            # TODO: Parse the next comma-separated value...
            pass

        if self.current.kind != TokenKind.RBRACKET:
            raise Exception(f"expected `{TokenKind.RBRACKET.value}` (found `{self.current}`)")
        self.current = self.lexer.next_token()
        return result
```

Every time the loop condition is checked, anything other than a `]` indicates
that there are still more elements to parse. Inside the loop we add an
additional check for the `,` separator, but only if at least one value has
already been parsed:

```python
    def parse_list(self):
        if self.current.kind != TokenKind.LBRACKET:
            raise Exception(f"expected `{TokenKind.LBRACKET.value}` (found `{self.current}`)")
        self.current = self.lexer.next_token()

        result = []
        while self.current.kind != TokenKind.RBRACKET:
            if len(result) != 0:
                if self.current.kind != TokenKind.COMMA:
                    raise Exception(f"expected `{TokenKind.COMMA.value}` (found `{self.current}`)")
                self.current = self.lexer.next_token()
            # TODO: Parse the next comma-separated value...

        if self.current.kind != TokenKind.RBRACKET:
            raise Exception(f"expected `{TokenKind.RBRACKET.value}` (found `{self.current}`)")
        self.current = self.lexer.next_token()
        return result
```

And finally, after maybe-parsing the `,` separator, we attempt to parse a value
and append the value to our parse result list:

```python
    def parse_list(self):
        if self.current.kind != TokenKind.LBRACKET:
            raise Exception(f"expected `{TokenKind.LBRACKET.value}` (found `{self.current}`)")
        self.current = self.lexer.next_token()

        result = []
        while self.current.kind != TokenKind.RBRACKET:
            if len(result) != 0:
                if self.current.kind != TokenKind.COMMA:
                    raise Exception(f"expected `{TokenKind.COMMA.value}` (found `{self.current}`)")
                self.current = self.lexer.next_token()
            result.append(self.parse_value())

        if self.current.kind != TokenKind.RBRACKET:
            raise Exception(f"expected `{TokenKind.RBRACKET.value}` (found `{self.current}`)")
        self.current = self.lexer.next_token()
        return result
```

Of course if we were to test this right now we would run into an errors, since
we have not yet implemented `parse_value`:

```python
try:
    lexer = Lexer("[123,456,[789]]")
    parser = Parser(lexer)
    print(parser.parse_list())
except Exception as e:
    print(e)
```

```sh
$ python3 parser.py
TODO
```

A value is either a list or an integer, so our `parse_value` method will
forward to either `parse_list` or `parse_integer` depending on the current
token. If the current token is a `[` token then we know that the value being
parsed is a list. If the current token is an integer token then we know that
the value being parsed is an integer. If the current token is not a `[` token
or an integer token then we will raise an exception to indicate that a parse
error has occurred:

```python
    def parse_value(self):
        if self.current.kind == TokenKind.LBRACKET:
            return self.parse_list()

        if self.current.kind == TokenKind.INTEGER:
            return self.parse_integer()

        raise Exception(f"expected value (found `{self.current}`)")
```

We can see that calls to the `parse_value` and `parse_list` methods will both
produce the expected results now that both methods have been defined:

```python
lexer = Lexer("[123,456,[789]]")
parser = Parser(lexer)
print(parser.parse_list())

try:
    lexer = Lexer("[123,]")
    parser = Parser(lexer)
    print(parser.parse_list())
except Exception as e:
    print(e)

lexer = Lexer("[123]")
parser = Parser(lexer)
print(parser.parse_value())

lexer = Lexer("123")
parser = Parser(lexer)
print(parser.parse_value())
```

```sh
$ python3 parser.py
[123, 456, [789]]
expected value (found `]`)
[123]
123
```

The last method we have to implement is `parse_packet`. This method is similar
to `parse_list` with an additional check for the end-of-input token after the
list is parsed:

```python
    def parse_packet(self):
        value = self.parse_list()
        if self.current.kind != TokenKind.EOI:
            raise Exception(f"expected {TokenKind.EOI.value} (found `{self.current}`)")
        return value
```

The additional end-of-input check is used to make sure that a packet string
consists of only one list. If we were to omit this check then a packet string
such as `"[123,456][789]"` would be parsed into the Python list `[123, 456]`
and be considered valid even though the erroneous tokens `[`, `789`, and `]`
would still be unconsumed from the token stream. By checking for the
end-of-input token, any extra tokens after the packet list will produce a parse
error:

```python
lexer = Lexer("[123,456,[789]]")
parser = Parser(lexer)
print(parser.parse_packet())

try:
    lexer = Lexer("[123,456][789]")
    parser = Parser(lexer)
    print(parser.parse_packet())
except Exception as e:
    print(e)
```

```sh
$ python3 parser.py
[123, 456, [789]]
expected end-of-input (found `[`)
```

All together our `Parser` class takes the form:

```python
class Parser:
    def __init__(self, lexer):
        self.lexer = lexer
        self.current = lexer.next_token()

    def parse_packet(self):
        value = self.parse_list()
        if self.current.kind != TokenKind.EOI:
            raise Exception(f"expected {TokenKind.EOI.value} (found `{self.current}`)")
        return value

    def parse_value(self):
        if self.current.kind == TokenKind.LBRACKET:
            return self.parse_list()

        if self.current.kind == TokenKind.INTEGER:
            return self.parse_integer()

        raise Exception(f"expected value (found `{self.current}`)")

    def parse_list(self):
        if self.current.kind != TokenKind.LBRACKET:
            raise Exception(f"expected `{TokenKind.LBRACKET.value}` (found `{self.current}`)")
        self.current = self.lexer.next_token()

        result = []
        while self.current.kind != TokenKind.RBRACKET:
            if len(result) != 0:
                if self.current.kind != TokenKind.COMMA:
                    raise Exception(f"expected `{TokenKind.COMMA.value}` (found `{self.current}`)")
                self.current = self.lexer.next_token()
            result.append(self.parse_value())

        if self.current.kind != TokenKind.RBRACKET:
            raise Exception(f"expected `{TokenKind.RBRACKET.value}` (found `{self.current}`)")
        self.current = self.lexer.next_token()
        return result

    def parse_integer(self):
        if self.current.kind != TokenKind.INTEGER:
            raise Exception(f"expected {TokenKind.INTEGER.value} (found `{self.current}`)")
        result = int(self.current.text)
        self.current = self.lexer.next_token()
        return result
```

## Wrapping Up

Believe it or not, we are basically finished with our Advent of Code day 13
packet parser. We can add a nice `main` function to show how the argument
parser might be used in practice:

```python
def main(args):
    try:
        lexer = Lexer(args.packet)
        parser = Parser(lexer)
        print(parser.parse_packet())
    except Exception as e:
        print(f'"{s}" raised exception: {e}')


if __name__ == "__main__":
    ap = ArgumentParser(description="Parse a packet from an input string")
    ap.add_argument("packet")
    main(ap.parse_args())
```

```sh
$ python3 parser.py [1,2,3,[4,5],[[6]],[7,[8,9]]]
[1, 2, 3, [4, 5], [[6]], [7, [8, 9]]]
```

In this `main` function we create a lexer using the input string, then create a
parser using the lexer, then we call the `parse_packet` method on the parser to
transform the string into tokens into a list data structure containing the
values of the packet. There are about a bajillion things that were glossed over
and/or elided for the sake of simplicity in this blog post, but the lexer and
parser that we wrote is still relatively close in structure to the lexers and
parsers that one might find in the wild!

We went through things pretty quickly here, so I am going to recommend some
additional exercises for those who would like to explore further:

1. Currently our lexer and parser do not allow whitespace as part of the input
   string. In many real life applications (e.g. programming languages) we want
   to allow arbitrary whitespace between tokens. So the input strings
   `"[123,456]"`, `"[123, 456]"` and `"[ 123,    456 ]"` would all be parsed
   into the same data structure. Modify the lexer to skip over whitespace when
   parsing the next token. Hint: the lexer should be modified without needing
   to introduce a `TokenKind.WHITESPACE` token kind.
2. Modify the lexer and parser to allow strings as values. For the purposes of
   this exercise, a string consists of an opening single quote, followed by
   zero or more characters, followed by a closing single quote (e.g `'hello'`).
   After modifying the lexer and parser, your program should be able to parse
   the packet `"[123,'foo',[456,'bar','baz']]"`. Hint: traditionally a string
   is considered a single token, so start by adding `TokenKind.STRING` to the
   `TokenKind` enum and modify `Lexer.next_token` to parse a string if the
   current character is `'`.

The full source code for `parser.py` can be found
[here](/blog/2023-05-03-advent-of-code-2022-day-13-packet-parser/parser.py).

## Footnotes
[^1]:
The term "parsing" has a somewhat overloaded meaning here, referring to the
process of as a whole as well as the second phase of a lexing-parsing pipeline.

[^2]:
One would not typically read a sentence letter by letter such as "A-L-I-C-E
W-A-L-K-E-D T-O T-H-E S-T-O-R-E." Our human brains essentially have a builtin
lexer that automatically tokenizes English text into individual words (tokens).

[^3]:
["Buffalo buffalo Buffalo buffalo buffalo buffalo Buffalo buffalo"](https://en.wikipedia.org/wiki/Buffalo_buffalo_Buffalo_buffalo_buffalo_buffalo_Buffalo_buffalo)
is an English sentence that is grammatically correct, but incredibly difficult
to understand. Similarly,
["Colorless green ideas sleep furiously"](https://en.wikipedia.org/wiki/Colorless_green_ideas_sleep_furiously)
is an English sentence that is grammatically correct, but semantically
nonsensical.
