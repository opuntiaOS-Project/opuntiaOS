# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

class Message:
    def __init__(self, name, id, reply_id, decoder_magic, params, protected=False):
        self.name = name
        self.id = id
        self.reply_id = reply_id
        self.decoder_magic = decoder_magic
        self.params = params
        self.protected = protected


class Generator:

    def __init__(self):
        self.output = None

    def out(self, str, tabs=0):
        for i in range(tabs):
            self.output.write("    ")
        self.output.write(str)
        self.output.write("\n")

    def params_readable(self, params):
        res = ""
        if len(params) > 0:
            for i in params:
                res += "{0} {1},".format(i[0], i[1])
            res = res[:-1]
        return res

    def message_create_std_funcs(self, msg):
        self.out("int id() const override {{ return {0}; }}".format(msg.id), 1)
        self.out("int reply_id() const override {{ return {0}; }}".format(
            msg.reply_id), 1)
        if msg.protected:
            self.out("int key() const override { return m_key; }", 1)
        self.out("int decoder_magic() const override {{ return {0}; }}".format(
            msg.decoder_magic), 1)
        for i in msg.params:
            self.out(
                "{0} {1}() const {{ return m_{1}; }}".format(i[0], i[1]), 1)

    def message_create_vars(self, msg):
        if msg.protected:
            self.out("message_key_t m_key;", 1)
        for i in msg.params:
            self.out("{0} m_{1};".format(i[0], i[1]), 1)

    def message_create_constructor(self, msg):
        params = msg.params
        if msg.protected:
            params = [('message_key_t', 'key')] + msg.params
        res = "{0}({1})".format(msg.name, self.params_readable(params))
        if len(params) > 0:
            self.out(res, 1)
            sign = ':'
            for i in params:
                self.out("{0} m_{1}({1})".format(sign, i[1]), 2)
                sign = ','

            self.out("{", 1)
            self.out("}", 1)
        else:
            self.out(res+" {}", 1)

    def message_create_encoder(self, msg):
        self.out("EncodedMessage encode() const override".format(
            msg.decoder_magic), 1)
        self.out("{", 1)

        self.out("EncodedMessage buffer;", 2)
        self.out("Encoder::append(buffer, decoder_magic());", 2)
        self.out("Encoder::append(buffer, id());", 2)
        if msg.protected:
            self.out("Encoder::append(buffer, key());", 2)
        for i in msg.params:
            self.out("Encoder::append(buffer, m_{0});".format(i[1]), 2)

        self.out("return buffer;", 2)
        self.out("}", 1)

    def generate_message(self, msg):
        self.out("class {0} : public Message {{".format(msg.name))
        self.out("public:")
        self.message_create_constructor(msg)
        self.message_create_std_funcs(msg)
        self.message_create_encoder(msg)
        self.out("private:")
        self.message_create_vars(msg)
        self.out("};")
        self.out("")

    def decoder_create_vars(self, messages, offset=0):
        var_names = set()
        for (name, params) in messages.items():
            for i in params:
                if 'var_{0}'.format(i[1]) not in var_names:
                    self.out("{0} var_{1};".format(i[0], i[1]), offset)
                    var_names.add('var_{0}'.format(i[1]))

    def decoder_decode_message(self, msg, offset=0):
        params_str = ""
        if msg.protected:
            params_str = "secret_key, "
        for i in msg.params:
            params_str += "var_{0}, ".format(i[1])

        if len(params_str) > 0:
            params_str = params_str[:-2]
        for i in msg.params:
            self.out(
                "Encoder::decode(buf, decoded_msg_len, var_{0});".format(i[1]), offset)
        self.out("return new {0}({1});".format(msg.name, params_str), offset)

    def decoder_create_std_funcs(self, decoder):
        self.out("int magic() const {{ return {0}; }}".format(
            decoder.magic), 1)

    def decoder_create_decode(self, decoder):
        self.out(
            "std::unique_ptr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len) override", 1)
        self.out("{", 1)
        self.out("int msg_id, decoder_magic;", 2)
        self.out("size_t saved_dml = decoded_msg_len;", 2)
        self.out("Encoder::decode(buf, decoded_msg_len, decoder_magic);", 2)
        self.out("Encoder::decode(buf, decoded_msg_len, msg_id);", 2)
        self.out("if (magic() != decoder_magic) {", 2)
        self.out("decoded_msg_len = saved_dml;", 3)
        self.out("return nullptr;", 3)
        self.out("}", 2)

        if decoder.protected:
            self.out("message_key_t secret_key;", 2)
            self.out("Encoder::decode(buf, decoded_msg_len, secret_key);", 2)
            self.out("", 0)

        self.decoder_create_vars(decoder.messages, 2)

        unique_msg_id = 1
        self.out("", 2)
        self.out("switch(msg_id) {", 2)
        for (name, params) in decoder.messages.items():
            self.out("case {0}:".format(unique_msg_id), 2)
            # Here it doen't need to know the real reply_id, so we can put 0 here.
            self.decoder_decode_message(
                Message(name, unique_msg_id, 0, decoder.magic, params, decoder.protected), 3)
            unique_msg_id += 1

        self.out("default:", 2)
        self.out("decoded_msg_len = saved_dml;", 3)
        self.out("return nullptr;", 3)
        self.out("}", 2)
        self.out("}", 1)
        self.out("", 1)

    def decoder_create_handle(self, decoder):
        self.out("std::unique_ptr<Message> handle(const Message& msg) override", 1)
        self.out("{", 1)
        self.out("if (magic() != msg.decoder_magic()) {", 2)
        self.out("return nullptr;", 3)
        self.out("}", 2)

        unique_msg_id = 1
        self.out("", 2)
        self.out("switch(msg.id()) {", 2)
        for (name, params) in decoder.messages.items():
            if name in decoder.functions:
                self.out("case {0}:".format(unique_msg_id), 2)
                self.out(
                    "return handle(static_cast<const {0}&>(msg));".format(name), 3)

            unique_msg_id += 1

        self.out("default:", 2)
        self.out("return nullptr;", 3)
        self.out("}", 2)
        self.out("}", 1)
        self.out("", 1)

    def decoder_create_virtual_handle(self, decoder):
        for (accept, ret) in decoder.functions.items():
            self.out(
                "virtual std::unique_ptr<Message> handle(const {0}& msg) {{ return nullptr; }}".format(accept), 1)

    def generate_decoder(self, decoder):
        self.out("class {0} : public MessageDecoder {{".format(decoder.name))
        self.out("public:")
        self.out("{0}() {{}}".format(decoder.name), 1)
        self.decoder_create_std_funcs(decoder)
        self.decoder_create_decode(decoder)
        self.decoder_create_handle(decoder)
        self.decoder_create_virtual_handle(decoder)
        self.out("};")
        self.out("")

    def includes(self):
        self.out("// Auto generated with utils/ConnectionCompiler")
        self.out("// See .ipc file")
        self.out("")
        self.out("#pragma once")
        self.out("#include <libipc/Encoder.h>")
        self.out("#include <libipc/ClientConnection.h>")
        self.out("#include <libipc/ServerConnection.h>")
        self.out("#include <libg/Rect.h>")
        self.out("#include <libg/string.h>")
        self.out("#include <new>")
        self.out("")

    def generate(self, filename, decoders):
        self.output = open(filename, "w+")
        self.includes()
        for decoder in decoders:
            msgd = {}
            unique_msg_id = 1
            for (name, params) in decoder.messages.items():
                msgd[name] = unique_msg_id
                unique_msg_id += 1

            for (name, params) in decoder.messages.items():
                reply_name = decoder.functions.get(name, None)
                reply_id = -1
                if reply_name is not None:
                    reply_id = msgd[reply_name]
                self.generate_message(
                    Message(name, msgd[name], reply_id, decoder.magic, params, decoder.protected))

            self.generate_decoder(decoder)
        self.output.close()
