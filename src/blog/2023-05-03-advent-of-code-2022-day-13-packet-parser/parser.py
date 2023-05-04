from argparse import ArgumentParser
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
