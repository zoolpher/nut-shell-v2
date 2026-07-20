// parser.cpp

#include "../include/parser.hpp"
#include "../include/command.hpp"

#include <string>
#include <iostream>


/*
    [BUG] 1  
        input and output files are not being parsed correctly. 
        The parser is not recognizing the '<' and '>' symbols as input 
        and output redirection operators, respectively. 
        This is likely due to the fact that the parser is only checking 
        for spaces and not for these symbols when parsing the command line input.

        try this: echo "sort < names.txt" | ./test_parser
        >>> wsl
        >>> make
        >>> ./test_parser
        (\) nut-shell-$ echo "sort < names.txt" | ./test_parser
    

    [BUG] 2
        append and overWrite flags being reset to false immediately after use, 
        inside the parser — before the struct is even returned. 
        So by the time anyone (test printer, or later executor.cpp) looked at the struct, 
        both flags always showed false — even when >> or > was actually used.

        try this : ls -la >> out.txt
        >>> wsl
        >>> make
        >>> ./test_parser
        (\) nut-shell-$ ls -la >> out.txt

        Append Flag: false     <--- this flag is false where as it should be true
        Overwrite Flag: false
        Background Flag: false


    [BUG] 3
        '&&' is not recognized as a special operator.

        Right now the parser only understands a single '&' (background flag).
        When it sees '&&', it treats each '&' separately — the first '&' sets
        bgFlag = true, and the second '&' does the same thing again — instead
        of recognizing "&&" together as a distinct "run next command only if
        this one succeeds" operator.

        Example of the bug:
            input:  sleep 5 && ls -la &
            result: ONE single Command struct is created:
                    args = ["sleep", "5", "ls", "-la"]
                    bgFlag = true
            
            Everything after '&&' gets merged into the SAME command's args,
            instead of being split into a separate command that waits for
            'sleep 5' to succeed first. This is why running it actually
            passes "-la" as a bogus argument to sleep, causing:
                sleep: invalid option -- 'l'

        To fix this properly, the parser needs to:
        1. Detect '&&' as its own operator (two '&' characters in a row,
            not the same thing as a single background '&').
        2. Split into two separate Command structs, similar to how '|' does it.
        3. Track that the second command should only run if the first one
            exits successfully (this also requires executor.cpp to check
            exit status, which it doesn't do yet either).

*/

vector<Command> parseInput(const string &input) {
    Command cmd;
    vector<Command> commands;
    string buffer;

    size_t inSize = input.size();   // size of input command

    for(size_t i = 0; i < inSize; i++) {

        if (input[i] == ' ') {
            if (!buffer.empty()) {
                cmd.args.push_back(buffer);     // push buffer string into (args vector)
                buffer.clear();                 // empty the buffer after pushing the arsument 
            }
            
            // if buffer is empty we skip the space character and continue to the next iteration of the loop
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

            else {          // checks for '>' flag
                while (i+1 < inSize && input[i+1] == ' ') { i++; }  
                cmd.overWrite = true;   // set overWrite flag to true
            }      
            
            continue;       // skip the '>' character and continue to the next iteration of the loop
        }
                
        else if (input[i] == '<') {
            if (!buffer.empty()) {
                cmd.args.push_back(buffer);
                buffer.clear();
            }

            /*
                [FIX] (Bug 1): 
                old code peeked at i+1 and stopped ONE character
                too early, landing ON the space instead of past it.
                New approach: step forward first, then skip while current char is space.
            */
            i++;
            while (i < inSize && input[i] == ' ') { i++; }

            // now i points to next abphabetic character after '<'
            while (i < inSize && 
                input[i] != ' ' && 
                input[i] != '>' &&
                input[i] != '<' &&
                input[i] != '|' &&
                input[i] != '&'
            ) {
                buffer += input[i];
                i++;
            }

            /*
                [FIX] (edge case): 
                guard against '<' with no filename after it
            */
            if (buffer.empty()) {
                std::cerr << "Error: No input file specified after '<' operator.\n";
                continue;   // skip the '<' character and continue to the next iteration of the loop
            }
            else {
                cmd.input_file = buffer;
                buffer.clear();
            }

            /*
                [FIX] (Bug 1, part 2): 
                restored this i-- (was missing/commented out).
                Without it, the outer for-loop's automatic i++ skips PAST an
                unprocessed '>' / '|' / '&' right after the filename, so that
                symbol's branch never runs. Only decrement if i is still in
                bounds and sitting on a real unprocessed symbol.
            */
            if (
                input[i] == '>' || 
                input[i] == ' ' ||
                input[i] == '|' ||
                input[i] == '&'  
            ) i--;

            continue;
        }
        

        

        else if (input[i] == '|') {
        /* 
            '|' means: take the output of the command on the left, and feed 
            it directly as the input to the command on the right. They run at 
            the same time, connected by a data stream.
        */
            
            if (!buffer.empty()) {
                cmd.args.push_back(buffer);
                buffer.clear();
            } 

            commands.push_back(cmd);
            cmd = Command();     // reset the cmd struct for the next command
            continue;       // skip the '|' character and continue to the next iteration of the loop
        }



        // [FIX] Bug 3
        else if (input[i] == '&') {
            if (i+1 < inSize && input[i+1] == '&') {
                if (!buffer.empty()) {
                    cmd.args.push_back(buffer);
                    buffer.clear();
                }

                i++;
                commands.push_back(cmd);
                cmd = Command();
                cmd.runOnlyIfPrevSucceeded = true;
                continue;
            }
            else {
                if (!buffer.empty()) {
                    cmd.args.push_back(buffer);
                    buffer.clear();
                }
                cmd.bgFlag = true;
                commands.push_back(cmd);   // NEW — end this command
                cmd = Command();           // NEW — start a fresh one
                continue;
            }

        }

        buffer += input[i];
    }
    /*
        [FIX] (Bug 2): 
        moved this block outside the for-loop so it runs exactly
        once, after all characters are processed, using whatever buffer holds
        at that point (the trailing filename after '>' or '>>', or the last arg).
        
        FIX (Bug 2, part 2): removed cmd.append = false / cmd.overWrite = false.
        These flags are needed later by executor.cpp to know HOW to open the
        file (append vs overwrite). Resetting them here in the parser destroys
        information the executor still needs — parser's job is just to fill
        the struct correctly, not consume its own flags.
    */
    
    if (cmd.append) {
        cmd.output.append(buffer);      // append buffer string to output string
        // cmd.append = false;             // reset append flag to false
    }
    else if (cmd.overWrite) {
        cmd.output = buffer;            // overwrite output string with buffer string
        // cmd.overWrite = false;          // reset overWrite flag to false
    }
    else {
        if (!buffer.empty()) cmd.args.push_back(buffer);
    }

    commands.push_back(cmd);    // push the last (cmd struct) into the (commands vector)
    return commands;
}