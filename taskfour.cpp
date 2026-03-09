#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

// readInputFile Function: Reads system data from the input file including the number of processes,
// number of resource types, the existence vector E, allocation matrix C and request matrix R 
bool readInputFile(const string& filename,
                   int& numProcesses,
                   int& numResources,
                   vector<int>& E,
                   vector<vector<int>>& C,
                   vector<vector<int>>& R)
{
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file '" << filename << "'" << endl;
        return false;
    }

    file >> numProcesses >> numResources;

    E.resize(numResources);
    for (int j = 0; j < numResources; j++) {
        file >> E[j];
    }

    C.assign(numProcesses, vector<int>(numResources));
    for (int i = 0; i < numProcesses; i++) {
        for (int j = 0; j < numResources; j++) {
            file >> C[i][j];
        }
    }

    R.assign(numProcesses, vector<int>(numResources));
    for (int i = 0; i < numProcesses; i++) {
        for (int j = 0; j < numResources; j++) {
            file >> R[i][j];
        }
    }

    file.close();
    return true;
}

// computeAvailable Function: Calculates the available resources vector "A" by subtracting the allocated
// resources in matrix "C" from the total resources in vector "E"
vector<int> computeAvailable(int numProcesses, int numResources,
                              const vector<int>& E,
                              const vector<vector<int>>& C)
{
    vector<int> A(numResources);
    for (int j = 0; j < numResources; j++) {
        A[j] = E[j];
        for (int i = 0; i < numProcesses; i++) {
            A[j] -= C[i][j];
        }
    }
    return A;
}

// canRun Function: Checks whether a process can run by comparing its resource requests with the currently available resources in vector "W"
bool canRun(int process, int numResources,
            const vector<vector<int>>& R,
            const vector<int>& W)
{
    for (int j = 0; j < numResources; j++) {
        if (R[process][j] > W[j]) {
            return false;
        }
    }
    return true;
}

// Deadlock Detection Algorithm: Returns a list of deadlocked process indices
vector<int> detectDeadlock(int numProcesses, int numResources,
                            const vector<int>& E,
                            const vector<vector<int>>& C,
                            const vector<vector<int>>& R)
{
    // - Create available resources vector "W" through the computeAvailable() function call
    vector<int> W = computeAvailable(numProcesses, numResources, E, C);

    // - At first, all processes are marked as "not finished" 
    // ( a process is only marked as "finished" when its request vector R[i] can be satisfied by whatever is currently in vector "W" )
    vector<bool> finished(numProcesses, false);

    // - Find a process that is unmarked and whose request vector is <= to "W"
    // - When found, its finishing is simulated: its resources are released into the vector "W" and then it's marked as finished
    // - This loop repeats until there are none of these processes left
    bool progress = true;
    while (progress) {
        progress = false;
        for (int i = 0; i < numProcesses; i++) {
            if (!finished[i] && canRun(i, numResources, R, W)) {
                // - Simulate process i completing and releasing its resources
                for (int j = 0; j < numResources; j++) {
                    W[j] += C[i][j];
                }
                finished[i] = true;
                progress = true;
            }
        }
    }

    // - Any and all processes still not finished are considered to be "deadlocked"
    vector<int> deadlocked;
    for (int i = 0; i < numProcesses; i++) {
        if (!finished[i]) {
            deadlocked.push_back(i);
        }
    }

    return deadlocked;
}

// printMatrix Function: Prints a matrix with a label
void printMatrix(const string& label, const vector<vector<int>>& matrix,
                 int rows, int cols)
{
    cout << label << ":" << endl;
    for (int i = 0; i < rows; i++) {
        cout << "  P" << i << ": ";
        for (int j = 0; j < cols; j++) {
            cout << matrix[i][j];
            if (j < cols - 1) cout << "  ";
        }
        cout << endl;
    }
}

// printVector Function: Prints a vector with a label
void printVector(const string& label, const vector<int>& vec)
{
    cout << label << ": ";
    for (int i = 0; i < (int)vec.size(); i++) {
        cout << vec[i];
        if (i < (int)vec.size() - 1) cout << "  ";
    }
    cout << endl;
}

// Main Simulation Loop
int main() {
    string filename;
    cout << "Enter input filename: ";
    cin >> filename;

    int numProcesses, numResources;
    vector<int> E;
    vector<vector<int>> C, R;

    // - Read input from file
    if (!readInputFile(filename, numProcesses, numResources, E, C, R)) {
        return 1;
    }

    // - Display input data
    cout << "\n--- Input Data ---" << endl;
    cout << "Number of Processes  : " << numProcesses << endl;
    cout << "Number of Resource Types: " << numResources << endl;
    printVector("Existence Vector E", E);
    printVector("Available Vector A", computeAvailable(numProcesses, numResources, E, C));
    printMatrix("Allocation Matrix C", C, numProcesses, numResources);
    printMatrix("Request Matrix R   ", R, numProcesses, numResources);
    cout << endl;

    // - Run deadlock detection
    vector<int> deadlocked = detectDeadlock(numProcesses, numResources, E, C, R);

    // - Output results
    cout << "--- Deadlock Detection Result ---" << endl;
    if (deadlocked.empty()) {
        cout << "No deadlock detected. All processes can complete." << endl;
    } else {
        cout << "DEADLOCK DETECTED!" << endl;
        cout << "Deadlocked processes: ";
        for (int i = 0; i < (int)deadlocked.size(); i++) {
            cout << "P" << deadlocked[i];
            if (i < (int)deadlocked.size() - 1) cout << ", ";
        }
        cout << endl;
    }

    return 0;
}