#include <libipc/ClientConnection.h>
#include <libipc/ServerConnection.h>
#include <string.h>
#include <syscalls.h>

class ServerMsg1 : public Message {
public:
    ServerMsg1() {}
    ~ServerMsg1() {}

    int decoder_magic() const override
    {
        return 0x6;
    }

    int id() const override
    {
        return 1;
    }
    
    EncodedMessage encode() const override
    {
        EncodedMessage it;
        it.push_back(decoder_magic());
        it.push_back(id());
        return it;
    }
};

class ServerMessageDecoder : public MessageDecoder {
public:
    ServerMessageDecoder() {}
    ~ServerMessageDecoder() {}

    int magic() override
    {
        return 0x6;
    }

    unique_ptr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len) override
    {
        int mes_magic = buf[0];
        if (mes_magic != magic()) {
            decoded_msg_len = 0;
            return nullptr;
        }
        int id = buf[1];
        decoded_msg_len = 2;
        if (id == 1) {
            // unique_ptr<Message> res(new ServerMsg1());
            // char idd = (char)res->id()+'0';
            // write(1, &idd, 1);
            return new ServerMsg1();
        }
        return nullptr;
    }

    unique_ptr<Message> handle(Message& msg) override
    {
        if (msg.id() == 1) {
            write(1, "Handler msg 1", 13);
        }
        if (msg.id() == 2) {
            write(1, "Handler msg 2", 13);
        }
        return nullptr;
    }
};

class ClientMessageDecoder : public MessageDecoder {
public:
    ClientMessageDecoder() {}
    ~ClientMessageDecoder() {}

    int magic() override
    {
        return 0x7;
    }

    unique_ptr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len)
    {
        int mes_magic = buf[size];
        if (mes_magic == magic()) {
            decoded_msg_len = 0;
            return nullptr;
        }
        return nullptr;
    }

    unique_ptr<Message> handle(const Message& msg)
    {
        // if (msg.id() == 1) {
        //     write(0, "Handler msg 1", 13);
        // }
        // if (msg.id() == 2) {
        //     write(0, "Handler msg 2", 13);
        // }
    }
};

int main(int argc, char** argv)
{
    auto sd = ServerMessageDecoder();
    auto cd = ClientMessageDecoder();
    int fd;
    int rd = fork();
    if (rd) {
        for (int i = 0; i < 10000000; i++) {
            int a = (i * 4) / 3;
        }
        for (int i = 0; i < 10000000; i++) {
            int a = (i * 4) / 3;
        }
        for (int i = 0; i < 10000000; i++) {
            int a = (i * 4) / 3;
        }
        for (int i = 0; i < 10000000; i++) {
            int a = (i * 4) / 3;
        }
        for (int i = 0; i < 10000000; i++) {
            int a = (i * 4) / 3;
        }
        fd = socket(PF_LOCAL, 0, 0);
        if (fd > 0) {
            connect(fd, "/win.sock", 9);
        }
        auto connection = ClientConnection(fd, sd, cd);
        while (1) {
            connection.send_message(ServerMsg1());
            for (int i = 0; i < 10000000; i++) {
                int a = (i * 4) / 3;
            }
        }
    } else {
        fd = socket(PF_LOCAL, 0, 0);
        // creat("/win.sock", 0);
        bind(fd, "/win.sock", 9);
        auto connection = ServerConnection(fd, sd, cd);
        while (1) {
            connection.pump_messages();
        }
    }
    return 0;
}