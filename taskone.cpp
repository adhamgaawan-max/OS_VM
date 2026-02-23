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

// pause Function: Pauses shell execution until the user presses 'Enter"
void pauseCmd() {
 cout << "Press Enter to continue...";
 cin.ignore();
 cin.get();   

}

// run Function: Executes external programs entered by the user
void runCommand(vector<string> parts) {

    bool background = false;

    // - Detects background execution ( if the last character is "&" )
    if (!parts.empty() && parts.back() == "&") {
        background = true;
    // Remove "&" from arguments
        parts.pop_back();  
    }
    // - Child process creation
    pid_t pid = fork(); 

    if (pid == 0) {  

        // - Convert vector<string> to char* array for execvp
        vector<char *> argv;
        for (auto &p : parts)
            argv.push_back(&p[0]);

        argv.push_back(nullptr); 

        // - Replace child with requested program
        execvp(argv[0], argv.data()); 

        perror("exec");
        exit(1);

        // - Parent process
    } else if (pid > 0) {  

        // - Wait for child to finish if its not in the background
        if (!background)
            waitpid(pid, nullptr, 0);  

    } else {
        perror("fork");
    }
}

// main Loop
int main(int argc, char *argv[]) {

    // Option 1: Default input source is standard input from the user
    istream *input = &cin;
    ifstream batchFile;

    // Option 2: if a batch file is provided, switch the input source
    if (argc == 2) {
        batchFile.open(argv[1]);
        if (!batchFile) {
            cerr << "Cannot open batch file\n";
            return 1;
        }
        input = &batchFile;
    }

    string line;

    while (true) {

        // Zombie Process Cleanup: Clean up finished background child processes
        // WNOHANG makes "waitpid" return immediately if no child has exited
        while (waitpid(-1, nullptr, WNOHANG) > 0);

    
        if (input == &cin)
            showPrompt();

        // Read line of input
        if (!getline(*input, line))
            break;

        // Tokenization of input
        auto parts = splitLine(line);
        if (parts.empty())
            continue;

       
        // Save original STDIN and STDOUT
        // File descriptors:
        //  0 = standard input (STDIN)
        //  1 = standard output (STDOUT)
        // They are saved so they can be restored post redirection
        int savedStdout = dup(STDOUT_FILENO);
        int savedStdin  = dup(STDIN_FILENO);

        int fdIn = -1, fdOut = -1;
        bool error = false;

        // Scan for <, >, >>
        for (auto it = parts.begin(); it != parts.end();) {

            // Input redirection: command < file
            if (*it == "<") {
                fdIn = open((it + 1)->c_str(), O_RDONLY);
                if (fdIn < 0) { 
                    perror("open input"); 
                    error = true; 
                    break; 
                }

                // Replace standard input (fd 0) with file descriptor
                dup2(fdIn, STDIN_FILENO);
                close(fdIn);

                // Remove "< filename" from argument list
                it = parts.erase(it, it + 2);
            }


            // Output redirection: command > file OR >> file
            else if (*it == ">" || *it == ">>") {

                bool append = (*it == ">>");

                fdOut = open((it + 1)->c_str(),
                             O_WRONLY | O_CREAT |
                             (append ? O_APPEND : O_TRUNC),
                             0644);

                if (fdOut < 0) { perror("open output"); error = true; break; }

                // Replace standard output (fd 1) with file descriptor
                dup2(fdOut, STDOUT_FILENO);
                close(fdOut);

                // Remove "> filename" or ">> filename"
                it = parts.erase(it, it + 2);
            }

            else ++it;
        }

       // Command Execution
        if (!error && !parts.empty()) {

            string cmd = parts[0];

            // Built-in commands execution ( without child process)
            if (cmd == "quit") break;
            else if (cmd == "cd") cdCmd(parts);
            else if (cmd == "dir") dirCmd(parts);
            else if (cmd == "environ") envCmd();
            else if (cmd == "set") setCmd(parts);
            else if (cmd == "echo") echoCmd(parts);
            else if (cmd == "help") helpCmd();
            else if (cmd == "pause") pauseCmd();

            // For running external commands
            else runCommand(parts);   
        }

        // Restore original file descriptors
        dup2(savedStdout, STDOUT_FILENO);
        dup2(savedStdin, STDIN_FILENO);

        close(savedStdout);
        close(savedStdin);
    }

    return 0;
}
