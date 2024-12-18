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

## Render Bad Apple Using GCC error messages.
Find some way to make GCC error messages print out in some sort of Pixel format
on a monospace terminal. Construct C programs that produce the desired frame in
error messages when compiled. Profit.

## Render Mandelbrot Set using things other than the actual equation
Bezier curves, machine learning, or maybe something else. Basically try to
approximate the famous image of the Mandelbrot set without actually using the
correct math.

## Implementing Fixed Point Math in Sunder
+ https://github.com/gbmhunter/MFixedPoint
+ https://github.com/MikeLankamp/fpm
+ https://en.wikipedia.org/wiki/Libfixmath
+ https://github.com/burner/sweet.hpp/blob/master/fixed.hpp
+ https://sourceforge.net/p/fixedptc/code/ci/default/tree/

## Write about my 200 hour Skyrim playthrough
Honestly, PatricianTV does a way better job tearing this game to shreds, but it
would still be nice to talk about my burnout with the game, how it made me
dislike the open world Bethesda RPG. And how I will never another Elder Scrolls
or Fallout game from Bethesda again.

https://www.youtube.com/watch?v=dHP02JEC5H0

## N-in-1 Shower Product

Extrapolated version of how men have 2-in-1, 3-in-1, etc. Scale the number of
things in the shower product ridiculously high, but make sure nothing clashes.
