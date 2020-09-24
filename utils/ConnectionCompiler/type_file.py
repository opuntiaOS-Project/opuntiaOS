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

    class Special:
        EOF = 'EOF'