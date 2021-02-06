#!/usr/bin/env python3

import argparse
import random
import string


def plname() -> str:
    vowels = "aeiou"
    letters = list(random.sample(string.ascii_lowercase, random.randint(3, 4)))
    if set(vowels).isdisjoint(letters):
        letters[random.randrange(len(letters))] = random.choice(vowels)
    return "".join(letters).capitalize()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Generate programming language names"
    )
    parser.add_argument(
        "count",
        metavar="N",
        type=int,
        nargs="?",
        default=1,
        help="Number of names to output (default 1)",
    )
    args = parser.parse_args()
    for _ in range(args.count):
        print(plname())
