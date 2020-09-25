#include "WSConnection.h"

class WServerDecoder : public WindowServerDecoder {
public:
    WServerDecoder() {}
    ~WServerDecoder() {}
    
    using WindowServerDecoder::handle;
    virtual unique_ptr<Message> handle(const GreetMessage& msg) override
    {
        return new GreetMessageReply(0x7);
    }

};
