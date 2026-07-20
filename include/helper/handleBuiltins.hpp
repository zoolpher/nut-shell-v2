// handleBuiltins.hpp

#pragma once

#include "command.hpp"

struct BuiltinResult {
    bool isBuiltin  = false;
    int exitStatus  = 0;
};

BuiltinResult handleBuiltin(const Command &cmd);