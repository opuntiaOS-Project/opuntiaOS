# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

class Type:

    Word = 'WORD'

    class Number:
        Integer = 'INTEGER'
        Real = 'REAL'
        Boolean = 'BOOLEAN'

    class BinaryOperation:
        Plus = 'PLUS'
        Minus = 'MINUS'
        Mul = 'MUL'
        Div = 'DIV'

    class UnaryOperation:
        fill = 'FILL'
        Plus = 'PLUS'
        Minus = 'MINUS'
        Not = 'NOT'

    class Lang:
        LeftBracket = 'LeftBracket'
        RightBracket = 'RightBracket'
        Semi = 'SEMI'
        Dot = 'DOT'
        Comma = 'COMMA'
        Colon = 'COLON'

    class Reserved:
        Begin = 'BEGIN'
        End = 'END'
        Return = 'RETURN'
        Name = 'NAME'
        Magic = 'MAGIC'
        KeyProtected = 'KEYP'

    class Special:
        EOF = 'EOF'