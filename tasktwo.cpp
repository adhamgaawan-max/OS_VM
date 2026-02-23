#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>

using namespace std;

mutex mtx;


// Each individual thread stores its result here
vector<map<string, int>> results;

// toLower Function: Removes non alphabetical characters and converts letters to lowercase
string toLower(string word) {
    string result = "";
    for (int i = 0; i < word.length(); i++) {
        if (isalpha(word[i])) {
            result += tolower(word[i]);
        }
    }
    return result;
}

// countWords Function: 
// 1. Counts how many times each word appears in the lines assigned to a specific thread
// 2. It then prints the word counts for that segment 
// 3. It then saves the results in a shared structure so they can later be combined into the final word-frequency output
void countWords(int id, vector<string> lines) {
    map<string, int> localCount;

    for (int i = 0; i < lines.size(); i++) {
        stringstream ss(lines[i]);
        string word;
        while (ss >> word) {
            string cleaned = toLower(word);
            if (cleaned != "") {
                localCount[cleaned]++;
            }
        }
    }

    // Mutex Locking: To prevent threads from printing simultaneously
    mtx.lock();
    cout << "\n[Thread " << id << "] word counts for segment " << id << ":" << endl;
    for (auto it = localCount.begin(); it != localCount.end(); it++) {
        cout << "  " << it->first << ": " << it->second << endl;
    }
    mtx.unlock();

    
    mtx.lock();
    results.push_back(localCount);
    mtx.unlock();
}

int main() {
    string filename;
    int N;

    cout << "Enter filename: ";
    cin >> filename;
    cout << "Enter number of threads: ";
    cin >> N;

    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: could not open file" << endl;
        return 1;
    }

    vector<string> lines;
    string line;
    while (getline(file, line)) {
        lines.push_back(line);
    }
    file.close();

    if (N > lines.size()) {
        N = lines.size();
    }

    // Splitting the total lines into "N" segments - Round Robin Method
    vector<vector<string>> segments(N);
    for (int i = 0; i < lines.size(); i++) {
        segments[i % N].push_back(lines[i]);
    }

    // Creation of threads
    vector<thread> threads;
    for (int i = 0; i < N; i++) {
        threads.push_back(thread(countWords, i + 1, segments[i]));
    }

    // Waiting for threads to finish before joining
    for (int i = 0; i < threads.size(); i++) {
        threads[i].join();
    }

    // Merging of results
    cout << "\n------ Final Word Counts ------" << endl;
    map<string, int> finalCount;
    for (int i = 0; i < results.size(); i++) {
        for (auto it = results[i].begin(); it != results[i].end(); it++) {
            finalCount[it->first] += it->second;
        }
    }

    for (auto it = finalCount.begin(); it != finalCount.end(); it++) {
        cout << it->first << ": " << it->second << endl;
    }

    return 0;
}