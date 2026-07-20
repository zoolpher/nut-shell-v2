// test_parser.cpp

#include "../include/parser.hpp"

#include <iostream>
#include <string>      // has : getline()

using std::cout;
using std::cin; 
using std::string; 

int main() {

    string task;
         
    while(true) {
        cout<< "\033[1;92m" 
            << "(\\) nut-shell-$  " 
            << "\033[0m";   
            
        getline(cin, task);

        vector<Command> all_commands = parseInput(task);

        int i = 1;
        for (const auto &cmd : all_commands) {
            cout << "Command " << i << ": ";
            for (const auto &arg : cmd.args) {
                cout << arg << " ";
            }
            cout << "\nOutput: " << cmd.output 
                 << "\nInput File: " << cmd.input_file
                 << "\nAppend Flag: " << (cmd.append ? "true" : "false")
                 << "\nOverwrite Flag: " << (cmd.overWrite ? "true" : "false")
                 << "\nBackground Flag: " << (cmd.bgFlag ? "true" : "false")
                 << "\nRunOnlyIfPrevSucceeded Flag: " << (cmd.runOnlyIfPrevSucceeded ? "true" : "false")
                 << "\n\n";
            i++;
            
        }
            
        if (task == "exit") break;
    }

    return 0;
}