// command.hpp

#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

struct Command {
    vector<string> args;

    string output       =  "";
    string input_file   = "";

    bool append         = false;
    bool overWrite      = false;
    bool bgFlag         = false;
    
    bool runOnlyIfPrevSucceeded    = false;    // [FIX] Bug 3 in src/parser.cpp
};


