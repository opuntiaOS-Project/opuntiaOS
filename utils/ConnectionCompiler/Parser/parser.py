from Lexer.lexer import Lexer
from token import Token
from type_file import Type
from connection import Connection

class Parser:

    def set_code_lines(self, code: [str]):
        self.lexer = Lexer(code)
        self.token = Token()
        self.current_token_id = -1
        self.read_tokens = 0
        self.lexer_rich_the_end = 0
        self.tokens = []
        self.next_token()

    def next_token(self):
        self.current_token_id += 1
        self.token = self.get_token_at(self.current_token_id)
        print(self.token)

    def get_token_at(self, pos):
        if pos >= self.read_tokens and not self.lexer_rich_the_end:
            for i in range(pos - self.read_tokens + 1):
                self.tokens.append(self.lexer.next_token())
                self.read_tokens += 1
                if self.tokens[-1].type == Type.Special.EOF:
                    self.lexer_rich_the_end = True
                    break

        return self.tokens[min(pos, self.read_tokens - 1)]

    def is_nth(self, type_of_token, n):
        if isinstance(type_of_token, list):
            for type_of_cur_token in type_of_token:
                if self.get_token_at(self.current_token_id+n).type == type_of_cur_token:
                    return True
            return False
        return self.get_token_at(self.current_token_id+n).type == type_of_token

    def is_next(self, type_of_token):
        if isinstance(type_of_token, list):
            for type_of_cur_token in type_of_token:
                if self.token.type == type_of_cur_token:
                    return True
            return False
        return self.token.type == type_of_token

    def must_next(self, type_of_token):
        if not self.is_next(type_of_token):
            print("{0} is not {1}".format(self.token, type_of_token))
            exit(1)

    def __init__(self):
        pass

    def eat_name(self):
        self.must_next(Type.Reserved.Name)
        self.next_token()
        self.must_next(Type.Lang.Colon)
        self.next_token()
        self.must_next(Type.Word)
        res = self.token.value
        self.next_token()
        return res

    def eat_protected(self):
        if (self.is_next(Type.Reserved.KeyProtected)):
            self.next_token()
            return True
        return False

    def eat_magic(self):
        self.must_next(Type.Reserved.Magic)
        self.next_token()
        self.must_next(Type.Lang.Colon)
        self.next_token()
        self.must_next(Type.Number.Integer)
        res = self.token.value
        self.next_token()
        return res

    def eat_params(self):
        params = []
        self.must_next(Type.Lang.LeftBracket)
        self.next_token()
        while not self.is_next(Type.Lang.RightBracket):
            self.must_next(Type.Word)
            typ = self.token.value
            self.next_token()

            self.must_next(Type.Word)
            nam = self.token.value
            self.next_token()

            params.append([typ, nam])

            if self.is_next(Type.Lang.Comma):
                self.must_next(Type.Lang.Comma)
                self.next_token()

        self.must_next(Type.Lang.RightBracket)
        self.next_token()
        return params


    def eat_message(self, decoder):
        self.must_next(Type.Word)
        msgname = self.token.value
        self.next_token()
        decoder.add_message(msgname, self.eat_params())
        return msgname

    def eat_function(self, decoder):
        ms1 = self.eat_message(decoder)
        ms2 = None

        if self.is_next(Type.Reserved.Return):
            self.must_next(Type.Reserved.Return)
            self.next_token()
            ms2 = self.eat_message(decoder)

        decoder.add_function(ms1, ms2)
    
    def eat_decoder(self):
        self.must_next(Type.Reserved.Begin)
        self.next_token()
        is_protected = self.eat_protected()
        decoder = Connection(self.eat_name(), self.eat_magic(), is_protected)
        while not self.is_next(Type.Reserved.End):
            print(self.eat_function(decoder))
        self.must_next(Type.Reserved.End)
        self.next_token()
        return decoder

    def parse(self):
        decoders = []
        while self.is_next(Type.Reserved.Begin):
            decoders.append(self.eat_decoder())
        return decoders