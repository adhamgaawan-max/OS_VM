#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <cstring>
#include <cstdlib>
#include <fstream>

using namespace std;

extern char **environ;

// Tokenizer Helper Function: Parses user input into tokens and returns a vector of them
vector<string> splitLine(const string &line){
    vector<string> parts;
    istringstream ss(line);
    string word;

    while(ss >> word)
    parts.push_back(word);

    return parts;
}

// Prompt Function: Displays the current working directory as the shell prompt
void showPrompt() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    cout << cwd << " $ ";
}

// cd Function: Implements the built in "cd" (change directory) command
void cdCmd(const vector<string> &args) {
    // - If no argument is given, prints the current directory
    if (args.size() == 1) { 
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        cout << cwd << endl;
        return;
    }

    // - If an argument is given, attempts to change to its directory
    if (chdir(args[1].c_str()) != 0) perror("cd");

    else {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        // - After the directory is changed, the PWD environment variable is updated
        setenv("PWD", cwd, 1); 
    }
}


// dir Function: Implements the built-in "dir" (list directory) command
void dirCmd(const vector<string> &args) {
    // - Check if a directory argument was provided by the user
    string path;
    if (args.size() > 1) {
        // - If an argument is given, use it as path
        path = args[1];
    } else {
        // - If no argument is given, default to the current working directory "."
        path = ".";
    }
    // - Attempt to open the directory stream for the path
    DIR *dir = opendir(path.c_str());
    if (!dir) { 
        perror("dir"); 
        return; 
    }

    // - Iterate through the directory entries and print them
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        cout << entry->d_name << endl;
    }

    closedir(dir);
}

// environ Function: Displays all environment variables available to the shell
void envCmd() {
    // - Iterates through the global 'environ' array and prints each variable; 'environ' is a global array of C-strings (NAME=VALUE format)
    for (char **env = environ; *env != nullptr; env++)
        cout << *env << endl;  
}

// set Function: Creates or updates an environment variable
void setCmd(const vector<string> &args) {

    // - Number of arguments check: the command "set", variable name, variable value
    if (args.size() < 3) {
        cerr << "Usage: set VARIABLE VALUE\n";
        return;
    }

    // - Creates or updates the environment variable based on the last 2 arguments
    setenv(args[1].c_str(), args[2].c_str(), 1);
}

// echo Function: Prints user-provided arguments to the console
void echoCmd(const vector<string> &args) {

    // - Print each argument except the first one ("echo")
    for (size_t i = 1; i < args.size(); i++)
        cout << args[i] << " ";

    cout << endl; 
}

// help Function: Displays the shell help manual using the 'more' paging program
void helpCmd() {
   const char *manual =
"==========================================================\n"
"                    MYSHELL - HELP GUIDE                  \n"
"==========================================================\n\n"
"INTERNAL COMMANDS\n"
"----------------------------------------------------------\n"
"cd [directory]\n"
"    Changes the current working directory.\n"
"    If no directory is provided, the current path is shown.\n\n"
"dir [directory]\n"
"    Displays the contents of the specified directory.\n"
"    If no directory is provided, the current one is used.\n\n"
"env\n"
"    Displays all environment variables available to the shell.\n\n"
"set [variable] [value]\n"
"    Creates or updates an environment variable.\n\n"
"echo [text]\n"
"    Prints the given text to the screen.\n\n"
"help\n"
"    Displays this help guide page by page.\n\n"
"pause\n"
"    Pauses execution until the user presses Enter.\n\n"
"quit\n"
"    Exits the shell.\n\n"
"EXTERNAL PROGRAM EXECUTION\n"
"----------------------------------------------------------\n"
"If a command is not built into the shell, it is treated\n"
"as an external program. The shell creates a child process\n"
"to execute it.\n\n"
"I/O REDIRECTION\n"
"----------------------------------------------------------\n"
"Supports input (<), output (>), and append (>>).\n"
"Example:\n"
"    program < input.txt > output.txt\n\n"
"BACKGROUND EXECUTION\n"
"----------------------------------------------------------\n"
"Adding '&' at the end of a command runs it in the background,\n"
"allowing the shell to accept new commands immediately.\n"
"==========================================================\n";

// - Creates a pipe: fd[0] = read end, fd[1] = write end
int fd[2];
pipe(fd);

// - Child Process is created
if (fork() == 0) { 
    // Redirect pipe input to standard input
    dup2(fd[0], STDIN_FILENO);
    // Close unused "write" end
    close(fd[1]);
    execlp("more", "more", nullptr);
    exit(0);
    } 
// - Parent Process
else { 
     // Close unused "read" end
        close(fd[0]);
    // Send manual text into pipe
        write(fd[1], manual, strlen(manual));
    // Close write end after sending data
        close(fd[1]);
    // Wait for child process to finish
        wait(nullptr);
    }
}