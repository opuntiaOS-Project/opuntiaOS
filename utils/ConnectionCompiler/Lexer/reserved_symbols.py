from type_file import Type

reserved_symbols = {
    '(': Type.Lang.LeftBracket,
    ')': Type.Lang.RightBracket,
    ';': Type.Lang.Semi,
    '.': Type.Lang.Dot,
    ',': Type.Lang.Comma,
    ':': Type.Lang.Colon,

    '=>': Type.Reserved.Return,

    '{': Type.Reserved.Begin,
    '}': Type.Reserved.End,
}

reserved_words = {
    'NAME': Type.Reserved.Name,
    'MAGIC': Type.Reserved.Magic,
}

available_var_types = [Type.Number.Integer, Type.Number.Real, Type.Number.Boolean]