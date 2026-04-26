#pragma once

#include "TAny.h"

struct NodeBase {
    virtual ~NodeBase() = default;
    virtual void Execute() = 0;
    virtual Any& GetRawResult() = 0;
    virtual bool WasMoved() const = 0;
    virtual void MarkAsMoved() = 0;
};