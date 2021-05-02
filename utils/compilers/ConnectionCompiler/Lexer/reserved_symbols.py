# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

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
    'KEYPROTECTED': Type.Reserved.KeyProtected,
}

available_var_types = [Type.Number.Integer,
                       Type.Number.Real, Type.Number.Boolean]
