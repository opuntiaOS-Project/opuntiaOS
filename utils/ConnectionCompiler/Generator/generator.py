class Message:
    def __init__(self, name, id, decoder_magic, params):
        self.name = name
        self.id = id
        self.decoder_magic = decoder_magic
        self.params = params

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
        self.out("int decoder_magic() const override {{ return {0}; }}".format(msg.decoder_magic), 1)
        for i in msg.params:
            self.out("{0} {1}() const {{ return m_{1}; }}".format(i[0], i[1]), 1)

    def message_create_vars(self, msg):
        for i in msg.params:
            self.out("{0} m_{1};".format(i[0], i[1]), 1)

    def message_create_constructor(self, msg):
        res = "{0}({1})".format(msg.name, self.params_readable(msg.params))
        if len(msg.params) > 0:
            self.out(res, 1)
            sign = ':'
            for i in msg.params:
                self.out("{0} m_{1}({1})".format(sign, i[1]), 2)
                sign = ','
        
            self.out("{", 1)
            self.out("}", 1)
        else:
            self.out(res+" {}", 1)

    def message_create_encoder(self, msg):
        self.out("EncodedMessage encode() const override".format(msg.decoder_magic), 1)
        self.out("{", 1)
        
        self.out("EncodedMessage buffer;", 2)
        self.out("Encoder::append(buffer, decoder_magic());", 2)
        self.out("Encoder::append(buffer, id());", 2)
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

    def decoder_decode_message(self, msg, offset = 0):
        params_str = ""
        for i in msg.params:
            self.out("{0} var_{1};".format(i[0], i[1]), offset)
            params_str += "var_{0}, ".format(i[1])

        if len(msg.params) > 0:
            params_str = params_str[:-2]
        for i in msg.params:
            self.out("Encoder::decode(buf, decoded_msg_len, var_{0});".format(i[1]), offset)
        self.out("return new {0}({1});".format(msg.name, params_str), offset)

    def decoder_create_std_funcs(self, decoder):
        self.out("int magic() const {{ return {0}; }}".format(decoder.magic), 1)

    def decoder_create_decode(self, decoder):
        self.out("unique_ptr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len) override", 1)
        self.out("{", 1)
        self.out("int msg_id, decoder_magic;", 2)
        self.out("Encoder::decode(buf, decoded_msg_len, decoder_magic);", 2)
        self.out("Encoder::decode(buf, decoded_msg_len, msg_id);", 2)
        self.out("if (magic() != decoder_magic) {", 2)
        self.out("return nullptr;", 3)
        self.out("}", 2)

        unique_msg_id = 1
        self.out("", 2)
        self.out("switch(msg_id) {", 2)
        for (name,params) in decoder.messages.items():
            self.out("case {0}:".format(unique_msg_id), 2)
            self.decoder_decode_message(Message(name, unique_msg_id, decoder.magic, params), 3)
            unique_msg_id += 1
            
        self.out("default:", 2)
        self.out("return nullptr;", 3)
        self.out("}", 2)
        self.out("}", 1)

    def generate_decoder(self, decoder):
        self.out("class {0} : public MessageDecoder {{".format(decoder.name))
        self.out("public:")
        self.out("{0}() {{}}".format(decoder.name), 1)
        self.decoder_create_std_funcs(decoder)
        self.decoder_create_decode(decoder)
        self.out("};")

    def includes(self):
        self.out("#pragma once")
        self.out("#include <libipc/Encoder.h>")
        self.out("#include <libipc/ClientConnection.h>")
        self.out("#include <libipc/ServerConnection.h>")
        self.out("#include <malloc.h>")

    def generate(self, filename, decoder):
        self.output = open(filename, "w+")
        self.includes()
        unique_msg_id = 1
        for (name,params) in decoder.messages.items():
            self.generate_message(Message(name, unique_msg_id, decoder.magic, params))
            unique_msg_id += 1

        self.generate_decoder(decoder)
        self.output.close()
        