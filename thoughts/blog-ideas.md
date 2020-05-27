Blog Post Ideas
===============

## The Elixer Effect
Talk about the `elixer` item from FFIII and how single-use-non-rechargable items
encourage players never to use them.
Talk about BoTW and how high-damage weapons never got used because I was saving
them for a fight that never came.

## Typesafe Return Status
Wrap an `int` in a struct and have a macro `ERROR(x)` that will extract the
that int.
If zero implies success then `if (ERROR(status))` will become an explicit
error check *and* you can reuse the macro for `strerror`-style error number
extraction.
Could also have a macro/function `MAKE_STATUS`/`make_status` that will construct
this struct.
