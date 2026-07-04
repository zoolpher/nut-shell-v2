// parser.cpp

#include "parser.hpp"
#include "command.hpp"

#include <string>

Command parseInput(const string &input) {
    Command cmd;
    string buffer;

    size_t inSize = input.size();   // size of input command

    for(size_t i = 0; i < inSize; i++) {

        if (input[i] == ' ') {
            cmd.args.push_back(buffer);     // push buffer string into (args vector)
            buffer.clear();                 // empty the buffer after pushing the arsument 
            continue;
        }
        
        else if (input[i] == '>') {
            if (!buffer.empty()) {      // if buffer is not empty we push it's value to (args vector)
                cmd.args.push_back(buffer);
                buffer.clear();
            }

            if (i+1 < inSize && input[i+1] == '>') {    // checks for '>>' flag
                i++;        // increment i to skip the second '>' character
                while (i+1 < inSize && input[i+1] == ' ') { i++; }  
                cmd.append = true;      // set append flag to true
            }   

            else {          
                while (i+1 < inSize && input[i+1] == ' ') { i++; }  
                cmd.overWrite = true;   // set overWrite flag to true
            }      
            
            continue;       // skip the '>' character and continue to the next iteration of the loop
        }
                
        else if (input[i] == '<') {}
        
        else if (input[i] == '|') {}
        
        else if (input[i] == '&') {}

        buffer += input[i];

        if (i == inSize - 1) {
            if (cmd.append) {
                cmd.output.append(buffer);      // append buffer string to output string
                cmd.append = false;             // reset append flag to false
            }
            else if (cmd.overWrite) {
                cmd.output = buffer;            // overwrite output string with buffer string
                cmd.overWrite = false;          // reset overWrite flag to false
            }
            else  {
                cmd.args.push_back(buffer);     // push buffer string into (args vector)
            }
        }
    }
    return cmd;
}