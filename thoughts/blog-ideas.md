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

## Cron Job to Automate Being a Good Romatic Partner
Automated email reminders to initiate and set up dates.
Use two lines in my crontab that send me email reminders every X and Y days
where X and Y are relatively prime so that the time at which dates are scheduled
doesn't *just* become painfully predictable monthly or bi-monthly event.

Automated text reminders to ask partner about their day at 18:00 every weekday.

## Elseless Programming
Reference that Golang talk about the things in Go that the speaker never uses.

## Golden Testing in C99
Use autil library as example.
Or put together a minimal Makefile + boilerplate code to get started.

## Pre C99 static assert
Use the hackey enum trick.

## Golflibc
ANSI C standards compliant libc that uses the fewest number of possible ASCII
characters.

## Metronome-Lang
Esolang that has random effects for some core keywords, where the initial RNG
seed is based the hash of the program. Allows random-effects to happen while
programming while still being deterministic.

## Electoral College / House of Representative Apportionment Calculations
See:
+ https://en.wikipedia.org/wiki/United_States_congressional_apportionment
+ https://en.wikipedia.org/wiki/Huntington%E2%80%93Hill_method
+ https://www.youtube.com/watch?v=6JN4RI7nkes

## Reverse iteration trick in C
```
for (size_t i = thing_count; i--;) {
  // code...
}
```

## Render Bad Apple Using GCC error messages.
Find some way to make GCC error messages print out in some sort of Pixel format
on a monospace terminal. Construct C programs that produce the desired frame in
error messages when compiled. Profit.
