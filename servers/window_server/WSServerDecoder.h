#pragma once
#include "Compositor.h"

class WServerDecoder : public WindowServerDecoder {
public:
    WServerDecoder() { }
    ~WServerDecoder() { }

    using WindowServerDecoder::handle;
    virtual UniquePtr<Message> handle(const GreetMessage& msg) override;
    virtual UniquePtr<Message> handle(const CreateWindowMessage& msg) override;
    virtual UniquePtr<Message> handle(const SetBufferMessage& msg) override;
};
