// executor.cpp

#include "../include/executor.hpp"
#include "../include/helper/handleBuiltins.hpp"

#include <iostream>
#include <unistd.h> // fork(), execvp();
#include <sys/wait.h>   // wait()
#include <string>
#include <fcntl.h>  // open() and it's flags

using std::string;

static vector<pid_t> bgPids;

void executor(vector<Command> commands) {
    if (commands.empty()) return;



    // non-blocking check — report any bg jobs that finished since last call
    for (auto it = bgPids.begin(); it != bgPids.end(); ) {
        int status;
        pid_t result = waitpid(*it, &status, WNOHANG);
        if (result > 0) {
            std::cout << "[bg] Process " << result << " done\n";
            it = bgPids.erase(it);
        } else {
            ++it;
        }
    }

    int prevReadEnd = -1;   // -1 means "no pipe to read from yet" (true for the first command)
    int prevExitStatus = 0; // to check if previous command ram successfully
    
    
    /*
    all_pid stores processIDs of all the child proecsses ii 
    they are connected by pipes
    
    and we pause each parent process one by one using child 
    processes' processID one after another after we have 
    forked all the child processes 
    */
    vector<int> all_pid;
    
    
    
    // Proceed if commands array is not empty
    for (size_t i = 0; i < commands.size(); i++) {
       
        // commands[i] is a srtuct of Command type
        if (commands[i].args.empty()) continue; // move to next command in vector<Command>
       


        /*
            updating valued of "readEndForThisCommand" to keep a 
            record of previous reader for the current child
        */
        int readEndForThisCommand = prevReadEnd;



        // c_type empty array of size 2
        int pipefd[2];      



        /*
            pipe() creates two connected file descriptors in one call:
            pipefd[0] — the read end
            pipefd[1] — the write end

            Whatever we push(write) into pipefd[1], we can pull(read)
            back out of pipefd[0].

            pipe() returns:
            0  -> success
            -1 -> failure

            pipefd[0] -> holds the actual fd number (assigned by the OS,
            could be 3, 5, whatever) for the read end.
            pipefd[1] -> holds the actual fd number (assigned by the OS)
            for the write end.
        */
        if (i+1 < commands.size()) {    // create a pipe only if the next command exists
            pipe(pipefd);    // fills pipefd[0] and pipefd[1] with real fd numbers
            
            // DEBUG statement:
            // std::cout << "DEBUG: created pipe for i=" 
            //         << i 
            //         << " read=" 
            //         << pipefd[0] 
            //         << " write=" 
            //         << pipefd[1] 
            //         << "\n";

            /*
                Before overwriting prevReadEnd with THIS iteration's new
                read end, close the OLD read end from the PREVIOUS iteration.

                Why here, not earlier: the previous iteration's child already
                used dup2(prevReadEnd, 0) to copy that fd onto its own stdin.
                Once that copy exists, the parent's original copy of the old
                prevReadEnd serves no purpose — holding it open leaks a file
                descriptor and can prevent the pipe from signaling EOF properly.
            */
            if (prevReadEnd != -1) {
                close(prevReadEnd);     // close OLD read-end before overwriting
            }


            prevReadEnd = pipefd[0];    // iteration's read end, for the NEXT command to use
        }


        if (
            commands[i].runOnlyIfPrevSucceeded && 
            prevExitStatus != 0
        ) continue;



        /*
            Builtin command check like (cd,...)

            Some commands (like 'cd') can't run as a normal forked child process,
            because they need to change something about shell's own state
            (e.g. its current working directory). If run inside a child via
            fork()+execvp(), the change would only affect the child, and be lost
            the moment that child exits.

            handleBuiltin() checks if commands[i] is one of these special cases
            and handles it directly, in the parent, with no fork() at all.

            Return value meaning:
            -1 -> not a builtin, caller should fork()/execvp() as normal
            0  -> was a builtin, and it succeeded
            1  -> was a builtin, and it failed

            We still update prevExitStatus here (same as a normal forked command
            would via wait()), so that a later '&&' chain still works correctly
            even when one of the commands involved was a builtin.
        */
        BuiltinResult res = handleBuiltin(commands[i]);
        if (res.isBuiltin) {
            prevExitStatus = res.exitStatus;   // 0 = success, -1 = failure
            continue;   // skip fork() entirely, this command is done
        }


        
        /*
            fork()
            creates a child process
            - on success it returns processID of the child process
            - on failure returns -1
            - if we are inside child process, returns 0 
        */
        int processID = fork(); 
        
        

        if (processID == -1) {
            std::cout << "Could not execute command, child not created";
        }


        else if (processID == 0) {
            const char* c_args[static_cast<int>(commands[i].args.size() + 1)];
            
            
            
            int j = 0;
            for (const string& a : commands[i].args) {
                c_args[j] = a.c_str();
                j++;
            }
            c_args[j] = nullptr;


            // DEBUG statement:
            // std::cout << "DEBUG: c_args = [";
            // for (int k = 0; k < j; k++) std::cout << "'" << c_args[k] << "' ";
            // std::cout << "]\n";



            /*
                fd -> file descriptor (integer values)
                new fd from open starting typically at 3 

                Every process starts with these 3 file descriptors:
                0 -> stdin (keyboard)
                1 -> stdout (screen)
                2 -> error 
            */
            int fd;



            /*
                open() returns -> non-negative integer (0, 1, 2, 3, ...) 
                the new file descriptor, on success. 

                Open() returns -> -1 on failure
            */
            /*
                Output redirection ('>' or '>>')
                - append: open in append mode, writes go to end of file, existing content preserved
                - overWrite: open in truncate mode, existing content wiped, file starts fresh
                - O_CREAT: create the file if it doesn't already exist
                - fd 1 = stdout, so dup2(fd, 1) makes anything written to stdout
                actually go into this file instead of the screen
            */
            if (commands[i].append) {
                fd = open(commands[i].output.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (fd == -1) {
                    perror("nut-shell");
                    _exit(1);
                }
                else {
                    dup2(fd, 1);
                    close(fd);
                }
            }
            else if (commands[i].overWrite) {
                fd = open(commands[i].output.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("nut-shell");
                    _exit(1);
                }
                else {
                    dup2(fd, 1);
                    close(fd);
                }
            }



            /*
                Input redirection ('<')
                - O_RDONLY: file must already exist, we're only reading from it
                - no O_CREAT: if the file doesn't exist, open() fails (fd == -1)
                and we just skip redirection rather than creating an empty file
                - fd 0 = stdin, so dup2(fd, 0) makes the process read its input
                from this file instead of the keyboard
            */
            if (!commands[i].input_file.empty()) {
                // DEBUG statement:
                // std::cout << "DEBUG: opening input file [" << commands[i].input_file << "]\n";
                // fd = open(commands[i].input_file.c_str(), O_RDONLY);
                // std::cout << "DEBUG: fd = " << fd << "\n";
                
                fd = open(commands[i].input_file.c_str(), O_RDONLY);
                if (fd == -1) {
                    perror("nut-shell");
                    _exit(1);
                }
                else {
                    dup2(fd, 0);
                    close(fd);
                }
            }



            /*
                Read side of the pipe.

                - prevReadEnd holds the (read-end fd) of pipe the previous
                iteration created (or -1 if there was no previous pipe).

                - fd 0 = stdin, so dup2(prevReadEnd, 0) 
                take fd 0 (this process's stdin, currently pointing at the 
                keyboard) and make it instead point to wherever prevReadEnd points.

                - close(prevReadEnd) after dup2: we no longer need the original
                fd number once it's been copied onto fd 0.
            */
            else if (readEndForThisCommand != -1) {
                dup2(readEndForThisCommand, 0);

                // DEBUG statement:
                // std::cout << "DEBUG: child i=" 
                //         << i 
                //         << " redirecting stdin from readEndForThisCommand=" 
                //         << readEndForThisCommand 
                //         << "\n";
                close(readEndForThisCommand);
            }



            /*
                If execvp() succeeds: 
                it doesn't "return nothing" — it never returns at all. 
                The child process image is completely replaced by the 
                new program(ls, etc.), so the line execvp(...) and everything 
                after it (including your _exit(1)) simply never executes. 
                The process just becomes ls and runs to completion.
                
                If execvp() fails: 
                it returns -1 specifically — and execution continues to the 
                next line, which is your _exit(1).
                
                int execvp(
                    const char *file, 
                    char *const argv[]
                );
            */
            if (i+1 < commands.size()) {
                dup2(pipefd[1], 1); // redirect stdout (fd -> 1) into the pipe's write end (pipfd[1])
                
                // DEBUG statement:
                // std::cout << "DEBUG: child i=" 
                //         << i 
                //         << " redirecting stdout to pipefd[1]=" 
                //         << pipefd[1] 
                //         << "\n";
                close(pipefd[1]);
            }



            execvp(c_args[0], const_cast<char* const*>(c_args));
            
            
            /*
            If execvp() fails :
            Exit codes are just a convention, restricted to 0–255 
            (they're stored in an unsigned 8-bit value at the OS level). 
            0 conventionally means "success," and any non-zero value 
            means "something went wrong" 
            
            Why I used 1 ?
            1 is just the simplest, most common convention for 
            "generic failure" — separate from execvp()'s own -1 return value, 
            which is a completely different context (a function return value, 
            not a process exit code).
            */ 
            perror("nut-shell");
            _exit(1);
        }


        else if (processID > 0) {
            if (!commands[i].bgFlag) {
                all_pid.push_back(processID);
            } else {
                bgPids.push_back(processID);   // track instead of dropping
            }



            /*
                [DESIGN NOTE]:
                'cmd & && next' is actually INVALID syntax in real bash — tested on WSL,
                bash throws "syntax error near unexpected token `&&'". '&' already
                terminates a command list, so a '&&' right after it doesn't parse at all
                in real bash's grammar.

                nut-shell is more permissive: it accepts this input and interprets it,
                rather than rejecting it as a syntax error. Chosen behavior: backgrounding
                takes priority — if commands[i].bgFlag is true, skip the wait() entirely,
                even if the next command is '&&'-chained. This means 'next' runs
                immediately, without waiting to know if the backgrounded command
                succeeded (since it hasn't finished yet, there's no way to know).

                Tested: sleep 5 & && echo done
                -> "done" printed immediately, next prompt returned right away,
                instead of blocking for 5 seconds.

                Open item: no visibility into when a backgrounded job actually finishes
                (real bash would print something like "[1]+ Done sleep 5"). Not fixed —
                would require job control (tracking bg PIDs + periodic non-blocking
                waitpid(..., WNOHANG) checks).
            */
            if (
                !commands[i].bgFlag && i+1 < commands.size() && 
                commands[i+1].runOnlyIfPrevSucceeded
            ) {
                int status;
                waitpid(processID, &status, 0);
                if (WIFEXITED(status)) {
                    prevExitStatus = WEXITSTATUS(status);
                }
            }


            /*
                close the write end
                once the child process has been created and has it's own copy            
            */ 
            if (i+1 < commands.size()) close(pipefd[1]);
        }   
    }

    for (size_t i = 0; i < all_pid.size(); i++) wait(nullptr);
}