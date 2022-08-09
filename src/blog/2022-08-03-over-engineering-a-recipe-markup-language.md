Over Engineering a Recipe Markup Language
=========================================

A few years ago my grandmother gifted me small box filled with dozens of
hand-picked recipes for appetizers, main courses, deserts, soups, salads,
bread, beverages, pasta, pies, and so much more. This recipe box is among my
most prized possessions and represents one of the few areas where my
grandmother and I share a common interest. I am not a very skilled cook, but
the act of preparing food is something that I deeply enjoy.

Occasionally my partner and I will make a date out of cooking, sometimes using
a recipe from this recipe box, and other times grabbing a random recipe off of
the internet. Unfortunately, my partner and I have not been keeping very good
track of which recipes we have already tried and which recipes we would like to
make again. So a little while ago I had the idea to put together a recipe
website for our own version of a shared recipe box where we could write down
and store the recipes that we thought were worth holding on to. I figured we
could structure the recipe website similar to how I structure and build my
personal website: recipes would be written in some plain text markup language
and a script would be used to generate a full static site from those recipes.

There are a bajillion recipe data formats and markup languages[^1], but I
thought it would be cool to try making my own recipe language, because building
things is fun and because I have rampant [NIH
syndrome](https://en.wikipedia.org/wiki/Not_invented_here). So this past
weekend I sat down with the intention to put together a simple recipe language
and before I knew it I had created a full blown text-based functional language
that had well exceeded the scope of my original plan. Here is the example
document, a PB&J recipe, taken from the [project
repository](https://github.com/ashn-dot-dev/tart):

```
[run lib/recipe.tart]
[recipe Peanut Butter and Jelly]

The classic peanut butter and jelly sandwich.

[ingredients
    [- bread (2 slices)]
    [- peanut butter]
    [- jelly]]

[instructions
    [- Spread peanut butter on a slice of bread.]
    [- Spread jelly on the other slice of bread.]
    [- Put the two slices of bread together to create a sandwich filled with a
       mixture of peanut butter and jelly.]]
```

And here is the HTML generated from this document:

```html
<h1>Peanut Butter and Jelly</h1> The classic peanut butter and jelly sandwich. <h2>Ingredients</h2> <ul><li>bread (2 slices)</li><li>peanut butter</li><li>jelly</li></ul> <h2>Instructions</h2> <ol><li>Spread peanut butter on a slice of bread.</li><li>Spread jelly on the other slice of bread.</li><li>Put the two slices of bread together to create a sandwich filled with a mixture of peanut butter and jelly.</li></ol>
```

The language, which I have decided to call Tart, uses a Lisp-like syntax where
procedures are invoked using square brackets, `[procedure argument-1 argument-2
etc.]`, and bare words are considered atomic strings. The expression:

```
[recipe Peanut Butter and Jelly]
```

consists of a procedure `recipe` called with the string arguments `Peanut`,
`Butter`, `and`, and `Jelly`. The result of each top-level expression in Tart
is written as output, and in this case the top-level `recipe` invocation is
transformed into the `h1` tags and content seen in the generated text.

The bare words:

```
The classic peanut butter and jelly sandwich.
```

below the recipe procedure call are all atomic string expressions that are
written as output which we see just after the closing `h1` tag in the generated
text. Tart *reads* like a plain text markup language, but *behaves* more like a
functional programming language.

I should also mention that Tart allows the user to define custom procedures in
order to create domain-specific markup languages. The `run` procedure at the
top of the PB&J recipe is actually loading and executing another document which
contains the declarations for the `recipe`, `ingredients`, and `instructions`
procedures used for markup, as well the procedure `-` which is used to make the
arguments to `instructions` and `ingredients` look like a bullet point. Here is
the contents of `lib/recipe.tart`:

```
[let hx [lambda [level args...]
    [join ""
        [vector
            [cat <h [get level] >]
            [get args]
            [cat </h [get level] >]]]]]

[let h1 [lambda [args...] [hx 1 [get args]]]]
[let h2 [lambda [args...] [hx 2 [get args]]]]

[let li-vector [lambda [args]
    [join "" [vector <li> [get args] </li>]]]]
[let ol-vector [lambda [args]
    [let list-items [map [get li] [get args]]]
    [join "" [cat [vector <ol>] [get list-items] [vector </ol>]]]]]
[let ul-vector  [lambda [args]
    [let list-items [map [get li] [get args]]]
    [join "" [cat [vector <ul>] [get list-items] [vector </ul>]]]]]

[let li [lambda [args...] [li-vector [get args]]]]
[let ol [lambda [args...] [ol-vector [get args]]]]
[let ul [lambda [args...] [ul-vector [get args]]]]

[let recipe [lambda [args...] [h1 [get args]]]]

[let ingredients [lambda [args...]
    [string [h2 Ingredients] [ul-vector [get args]]]]]

[let instructions [lambda [args...]
    [string [h2 Instructions] [ol-vector [get args]]]]]

[let - [lambda [args...] [string [get args]]]]
```

There is a lot of functional programmy looking stuff here, and the [language
overview](https://github.com/ashn-dot-dev/tart/blob/main/overview.tart)
contains a explanation for how many of these constructs work, but describing
the semantics of Tart's ad hoc language definition is not the point of this
post. I just think its neat that the language allows a user to define their own
custom markup starting with just a handful of builtin procedures. Tart is
definitely not the first language to allow custom procedures[^2], but I was
surprised at how they sort of naturally came about once I started thinking of
Tart as an expression-oriented language.

Okay back to the original problem. In order to make a cute recipe website we
needed a recipe markup language, and now we have a recipe markup language.
Mission accomplished, right? Well not really. Let's take a step back for a
moment and look at the PB&J recipe again. We have a recipe name, recipe
description, list of ingredients, and instructions on how to prepare the
recipe. Most recipes follow this structure. Recipe websites, cook books, and
the index cards in my recipe box all have a recipe name, recipe description,
ingredients list, and cooking instructions. We *can* use Tart with the
`recipe.tart` library to standardize this structure, but I think it is all a
bit... much.

Do we really need an expression-oriented text-based language with, closures,
higher-order functions, and all that other functional programmy stuff just to
write out a recipe name, recipe description, ingredients list, and cooking
instructions? Definitely not. Moreover, compare the PB&J recipe written in Tart
to that same recipe written in Markdown:

```md
Peanut Butter and Jelly
=======================

The classic peanut butter and jelly sandwich.

## Ingredients

- bread (2 slices)
- peanut butter
- jelly

## Instructions

1. Spread peanut butter on a slice of bread.
2. Spread jelly on the other slice of bread.
3. Put the two slices of bread together to create a sandwich filled with a
   mixture of peanut butter and jelly.
```

I think the Markdown version looks much nicer as a text file. When I showed
both versions of the recipe to my partner, they also preferred the Markdown
version. So even though Tart was originally designed for a recipe website, I do
not think we are actually going to end up using the language for that purpose.

Tart was a lot of fun to work on. I do not really see myself doing much with
the project after this week, but I think the language, the interpreter, and the
recipe-to-HTML library in `recipe.tart` are all in a good spot. This was my
first time working on a language where plain text was the primary data
construct, and I appreciate the lessons learned from working in such unfamiliar
territory. Tart may be over engineered for the problem that it was originally
intended to solve, but the experience of developing that over engineered
solution was enjoyable nonetheless. 😊

## Footnotes
[^1]:
[Cooklang](https://cooklang.org/),
[Fathub TOML files](https://git.sr.ht/~martijnbraam/fathub-data),
the [Open Recipe Format](https://open-recipe-format.readthedocs.io/),
[hRecipe](http://microformats.org/wiki/hrecipe), and
the [schema.org Recipe schema](https://schema.org/Recipe) just to name a few.

[^2]:
Languages with custom commands such as [LaTeX](https://www.latex-project.org/)
have been around since before I was born.
