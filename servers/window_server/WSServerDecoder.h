#pragma once
#include "Compositor.h"

class WServerDecoder : public WindowServerDecoder {
public:
    WServerDecoder() { }
    ~WServerDecoder() { }

    using WindowServerDecoder::handle;
    virtual unique_ptr<Message> handle(const GreetMessage& msg) override;
    virtual unique_ptr<Message> handle(const CreateWindowMessage& msg) override;
    virtual unique_ptr<Message> handle(const SetBufferMessage& msg) override;
};
