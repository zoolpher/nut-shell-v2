// main.cpp

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
            
        if (task == "exit") break;
    }

    return 0;
}