// handleBuiltins.cpp

# include "../../include/helper/handleBuiltins.hpp"

#include <unistd.h>     // chdir()
#include <cstdlib>      // getenv()

BuiltinResult handleBuiltin(const Command &cmd) {
    BuiltinResult result;

    if (cmd.args[0] == "cd") {
        result.isBuiltin = true;

        /*
            check if the cmd.args has more than just one arguments or not
            but for this HOME has to be set as an environment variable 
                
                if (cmd.args.size() == 1)       -> go to (HOME)
                else if (cmd.args.size() > 1)   -> convert next agruments to c_str() type

                call chdir() for "path" we got above;
        */
        const char* path = (cmd.args.size() > 1) ? cmd.args[1].c_str() : getenv("HOME");
        if (path == nullptr) {
            result.exitStatus = -1;
            return result;
        }

        int ret = chdir(path);
        result.exitStatus = ret;
    }

    return result;
}