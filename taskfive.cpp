#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <fstream>
#include <iomanip>
using namespace std;

struct Process {
    int pid;
    int arrival;
    int burst;
    int waiting;
    int remaining; 
};

// printTable Helper Function: Prints the results of a scheduling algorithm in a formatted table
void printTable(vector<Process> procs) {
    cout << left << setw(8) << "PID" << setw(14) << "Arrival" << setw(12) << "Burst" << setw(14) << "Waiting" << endl;
    cout << "------------------------------------------------" << endl;
    for (int i = 0; i < procs.size(); i++) {
        cout << left << setw(8) << procs[i].pid << setw(14) << procs[i].arrival << setw(12) << procs[i].burst << setw(14) << procs[i].waiting << endl;
    }
}

// Sort Helper Functions 
bool sortByArrival(Process a, Process b) { return a.arrival < b.arrival; }
bool sortByBurst(Process a, Process b)   { return a.burst < b.burst; }

// fcfs Function: Simulates First Come First Served scheduling
// - Processes are executed in the order they arrive
double fcfs(vector<Process> procs) {
    // - Sort by arrival time so we process them in order
    sort(procs.begin(), procs.end(), sortByArrival);

    int time = 0;
    double totalWait = 0;

    for (int i = 0; i < procs.size(); i++) {
        // - If the CPU is idle before this process arrives, jump ahead to its arrival
        if (time < procs[i].arrival)
            time = procs[i].arrival;

        // - Waiting time is how long the process sat idle before getting the CPU
        procs[i].waiting = time - procs[i].arrival;
        time += procs[i].burst;
        totalWait += procs[i].waiting;
    }

    cout << "\n--- FCFS ---" << endl;
    printTable(procs);

    double avg = totalWait / procs.size();
    cout << "Average Waiting Time: " << avg << endl;
    return avg;
}

// sjf Function: Simulates Shortest Job First scheduling (Non-Preemptive)
// - Each time the CPU is free, the arrived process with the shortest burst time is chosen next
double sjf(vector<Process> procs) {
    int n = procs.size();
    vector<bool> done(n, false);
    int time = 0;
    double totalWait = 0;
    int finished = 0;

    while (finished < n) {
        // - Scan all processes to find the shortest arrived job that hasn't run yet
        int idx = -1;
        for (int i = 0; i < n; i++) {
            if (!done[i] && procs[i].arrival <= time) {
                if (idx == -1 || procs[i].burst < procs[idx].burst)
                    idx = i;
            }
        }

        // - No process is ready yet, advance time by 1
        if (idx == -1) {
            time++;
            continue;
        }

        procs[idx].waiting = time - procs[idx].arrival;
        time += procs[idx].burst;
        totalWait += procs[idx].waiting;
        done[idx] = true;
        finished++;
    }

    cout << "\n--- SJF ---" << endl;
    printTable(procs);

    double avg = totalWait / n;
    cout << "Average Waiting Time: " << avg << endl;
    return avg;
}

// roundRobin Function: Simulates Round Robin scheduling
// - Each process gets a fixed time slice (quantum); if it doesn't finish it goes back to the queue
double roundRobin(vector<Process> procs, int quantum) {
    int n = procs.size();
    sort(procs.begin(), procs.end(), sortByArrival);

    // - Initialise remaining burst times
    for (int i = 0; i < n; i++)
        procs[i].remaining = procs[i].burst;

    queue<int> rq;          
    vector<int> finishTime(n, 0);
    int time = 0;
    int finished = 0;
    int next = 0;           

    rq.push(next);
    next++;

    while (finished < n) {
        // - If the queue is empty the CPU is idle, jump to the next arriving process
        if (rq.empty()) {
            time = procs[next].arrival;
            rq.push(next);
            next++;
        }

        int i = rq.front();
        rq.pop();

        // - Run the process for either the quantum or whatever it has left, whichever is smaller
        int run = min(quantum, procs[i].remaining);
        procs[i].remaining -= run;
        time += run;

        // - Enqueue any processes that arrived during this time slice
        while (next < n && procs[next].arrival <= time) {
            rq.push(next);
            next++;
        }

        if (procs[i].remaining == 0) {
            finishTime[i] = time; 
            finished++;
        } else {
            rq.push(i); 
        }
    }

    // - Waiting time = finish time - arrival time - burst time
    for (int i = 0; i < n; i++)
        procs[i].waiting = finishTime[i] - procs[i].arrival - procs[i].burst;

    double totalWait = 0;
    for (int i = 0; i < n; i++)
        totalWait += procs[i].waiting;

    cout << "\n--- Round Robin (quantum = " << quantum << ") ---" << endl;
    printTable(procs);

    double avg = totalWait / n;
    cout << "Average Waiting Time: " << avg << endl;
    return avg;
}

// Main Simulation Loop
int main() {
    int n;
    cout << "Enter number of processes: ";
    cin >> n;

    vector<Process> processes(n);
    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        processes[i].waiting = 0;
        cout << "P" << i + 1 << " arrival time: ";
        cin >> processes[i].arrival;
        cout << "P" << i + 1 << " burst time: ";
        cin >> processes[i].burst;
        processes[i].remaining = processes[i].burst;
    }

    int quantum;
    cout << "Enter time quantum for Round Robin: ";
    cin >> quantum;

    double avgFCFS = fcfs(processes);
    double avgSJF  = sjf(processes);
    double avgRR   = roundRobin(processes, quantum);

    cout << "\n=== Summary ===" << endl;
    cout << "FCFS avg waiting time:        " << fixed << setprecision(2) << avgFCFS << endl;
    cout << "SJF avg waiting time:         " << fixed << setprecision(2) << avgSJF  << endl;
    cout << "Round Robin avg waiting time: " << fixed << setprecision(2) << avgRR   << endl;
    
    return 0;
}