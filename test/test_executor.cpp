// test_executor.cpp

#include "../include/parser.hpp"
#include "../include/executor.hpp"

#include <iostream>
#include <unistd.h>   // getcwd()
#include <climits>    // PATH_MAX
#include <readline/readline.h>  // getline()
#include <readline/history.h>   // add_history()

using std::cout;
using std::cin; 
using std::string; 

/*
    List of commands to check:

    ls -la
    ls -la > out.txt
    cat out.txt
    ls -la >> out.txt
    cat out.txt
    sort < names.txt
    pwd
    echo hello
    ls -la | grep txt
    cat < names.txt | sort > sorted.txt
    nonexistentcommand123
    ls -la &
    exit
*/

int main() {
         
    while(true) {
        // cout<< "\n\033[1;92m" 
        //     << "(\\) nut-shell-$  " 
        //     << "\033[0m";
        
        
        char cwd[PATH_MAX];

        string promptStr;
        
        // getcwd() -> gets current working dir.
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            promptStr = "\033[1;92m(\\) nut-shell:\033[0;34m" + string(cwd) + "\033[1;92m-$  \033[0m";
        } else {
            promptStr = "\033[1;92m(\\) nut-shell-$  \033[0m";
        }

        char* line = readline(promptStr.c_str());
        if (line == nullptr) break; // Ctrl+D / EOF safety, readline returns nullptr on EOF

        add_history(line);          // keeps history of all commands    

        string task(line);          // convert to std::string for parseInput()/comparisons

        vector<Command> all_commands = parseInput(task);

        executor(all_commands);
            
        if (task == "exit") {
            free(line);
            break;
        }

        free(line);   // free after every readline() call, it heap-allocates each time

    }

    return 0;
}