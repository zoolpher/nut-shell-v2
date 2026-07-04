// command.hpp

#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

struct Command {
    vector<string> args;

    string output   =  "";
    string input    = "";

    bool append     = false;
    bool overWrite  = false;
    bool bgFlag     = false;
};


