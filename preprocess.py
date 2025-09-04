#!/usr/bin/env python3

from abc import ABC, abstractmethod
from copy import copy
from dataclasses import dataclass
import argparse
import enum
import html
import re
import sys


class Node(ABC):
    @abstractmethod
    def __str__(self):
        raise NotImplementedError()


class Text(Node):
    def __init__(self, text: str):
        self.text = text

    def __str__(self):
        return self.text


class Tag(Node):
    def __init__(self, attributes: dict[str, str], children: list[Node]):
        self.attributes = attributes
        self.children = children

    def inner(self) -> str:
        return "".join(str(child) for child in self.children)

    def __str__(self):
        return f"<div>TODO: {self.__class__.__name__}</div>"


class TagImage(Tag):
    def __str__(self):
        attrs = " ".join(f'{k}="{v}"' for k, v in self.attributes.items())
        alt = html.escape(self.inner().strip().replace("\n", " "))
        return f'<img {attrs} alt="{alt}"/>'


class TagWave(Tag):
    def __str__(self):
        text = self.inner()
        characters = []
        for i, character in enumerate(text):
            if character == " ":
                characters.append(" ")
                continue

            delay = i * 0.15  # 150 ms delay per character
            characters.append(
                f'<span class="wave-character" style="animation-delay: {delay}s;">{html.escape(character)}</span>'
            )

        return f'<span class="wave-container">{"".join(characters)}</span>'


class TagDialogue(Tag):
    def __str__(self):
        character = self.attributes["character"].lower()
        portrait = self.attributes.get("portrait", "default.png")

        return f"""
<div class="dialogue-container" dialogue-character="{character.lower()}">
    <div class="dialogue-portrait">
        <img src="/characters/{character}/{portrait}" alt="{portrait} portrait"/>
    </div>
    <div class="dialogue-content">
        <div class="dialogue-name">{character}</div>
        <div>{self.inner().strip()}</div>
    </div>
</div>""".strip()


TAG_REGISTRY = {
    "Image": TagImage,
    "Wave": TagWave,
    "Dialogue": TagDialogue,
}


@dataclass
class SourceLocation:
    file: str
    line: int


@dataclass
class ParseError(Exception):
    location: SourceLocation
    message: str


class TokenKind(enum.Enum):
    EOF = "end-of-file"
    TEXT = "text"
    OPENING_TAG = "opening tag"
    CLOSING_TAG = "closing tag"

    def __str__(self) -> str:
        return self.value


@dataclass
class Token:
    location: SourceLocation
    kind: TokenKind
    text: str


# \{{
RE_ESCAPED_LBRACES = re.compile(r"\\\{\{")
# \}}
RE_ESCAPED_RBRACES = re.compile(r"\\\}\}")
# attribute-a attribute-b="value-b" attribute-c
RE_ATTRIBUTES = re.compile(r'(\w+)(?:="([^"]*)")?')
# {{TagName attribute-a attribute-b="value-b" attribute-c}}
RE_OPENING_TAG = re.compile(r"\{\{([A-Za-z]\w*)(?:\s+([^}]*?))?\}\}")
# {{/TagName}}
RE_CLOSING_TAG = re.compile(r"\{\{/([A-Za-z]\w*)\}\}")


class Lexer:
    def __init__(self, source: str, location: SourceLocation):
        self.source = source
        self.location = location
        self.position = 0

    def next_token(self) -> Token:
        start = self.position
        location = copy(self.location)

        if self._is_eof():
            return Token(location, TokenKind.EOF, "")

        if match := self._match_opening_tag():
            self.position += len(match.group(0))
            return Token(location, TokenKind.OPENING_TAG, match.group(0))

        if match := self._match_closing_tag():
            self.position += len(match.group(0))
            return Token(location, TokenKind.CLOSING_TAG, match.group(0))

        text = ""
        while True:
            if self._is_eof():
                break

            if self._match_opening_tag() or self._match_closing_tag():
                break

            if match := self._match_escaped_lbraces():
                text += "{{"
                self.position += len(match.group(0))
                continue

            if match := self._match_escaped_rbraces():
                text += "}}"
                self.position += len(match.group(0))
                continue

            text += self._remaining()[0]
            self.position += 1
        return Token(location, TokenKind.TEXT, text)

    def _is_eof(self) -> bool:
        return self.position >= len(self.source)

    def _remaining(self) -> str:
        return self.source[self.position :]

    def _match_escaped_lbraces(self):
        return RE_ESCAPED_LBRACES.match(self._remaining())

    def _match_escaped_rbraces(self):
        return RE_ESCAPED_RBRACES.match(self._remaining())

    def _match_opening_tag(self):
        return RE_OPENING_TAG.match(self._remaining())

    def _match_closing_tag(self):
        return RE_CLOSING_TAG.match(self._remaining())


class Parser:
    def __init__(self, lexer: Lexer):
        self.lexer: Lexer = lexer
        self.current = lexer.next_token()

    def parse(self) -> list[Node]:
        nodes: list[Node] = list()
        while self.current.kind != TokenKind.EOF:
            nodes.append(self._parse_node())
        return nodes

    def _parse_node(self) -> Node:
        if self.current.kind == TokenKind.TEXT:
            return self._parse_text()

        if self.current.kind == TokenKind.OPENING_TAG:
            return self._parse_tag()

        raise ParseError(
            self.current.location, f"expected text or tag, found {self.current.kind}"
        )

    def _parse_text(self) -> Text:
        return Text(self._expect_current(TokenKind.TEXT).text)

    def _parse_tag(self) -> Tag:
        opening_tag = self._expect_current(TokenKind.OPENING_TAG)
        match = RE_OPENING_TAG.match(opening_tag.text)
        assert match is not None
        name = match.group(1)
        attributes: dict[str, str] = dict()
        if match.group(2) is not None:
            for match in RE_ATTRIBUTES.finditer(match.group(2).strip()):
                k = match.group(1)
                v = match.group(2) if match.group(2) is not None else ""
                attributes[k] = v

        children: list[Node] = list()
        while not self._check_current(TokenKind.CLOSING_TAG):
            children.append(self._parse_node())

        closing_tag = self._expect_current(TokenKind.CLOSING_TAG)
        match = RE_CLOSING_TAG.match(closing_tag.text)
        assert match is not None
        if match.group(1) != name:
            raise ParseError(
                self.current.location,
                f"expected {TokenKind.CLOSING_TAG} `{name}`, found `{match.group(1)}`",
            )

        return TAG_REGISTRY[name](attributes, children)

    def _advance(self) -> None:
        self.current = self.lexer.next_token()

    def _check_current(self, kind: TokenKind) -> bool:
        return self.current.kind == kind

    def _expect_current(self, kind: TokenKind) -> Token:
        current = self.current
        if current.kind != kind:
            raise ParseError(current.location, f"expected {kind}, found {current}")
        self._advance()
        return current


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "file",
        nargs="?",
        type=argparse.FileType("r"),
        default=sys.stdin,
        help="Input file (use '-' to read from stdin)",
    )
    args = parser.parse_args()

    try:
        source = args.file.read()
        location = SourceLocation(args.file.name, 1)
        nodes = Parser(Lexer(source, location)).parse()
        for node in nodes:
            print(node, end="")

    except ParseError as e:
        print(
            f"[{e.location.file}:{e.location.line}] error: {e.message}", file=sys.stderr
        )
        sys.exit(1)


if __name__ == "__main__":
    main()
