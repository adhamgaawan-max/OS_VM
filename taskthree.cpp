#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
using namespace std;

// Frame Structure: to hold page number and aging register
struct Frame {
    int pageNumber;
    unsigned int age;
};

// Simulate Aging Function: returns the total number of page faults for a given number of frames
int simulateAging(const vector<int>& references, int numFrames) {
    vector<Frame> frames;
    int pageFaults = 0;

    for (int i = 0; i < (int)references.size(); i++) {
        int ref = references[i];
        bool found = false;

        // - Shifts all aging registers right by 1 bit
        for (int j = 0; j < (int)frames.size(); j++) {
            frames[j].age >>= 1;
        }

        // - Checks if page already exists in memory
        for (int j = 0; j < (int)frames.size(); j++) {
            if (frames[j].pageNumber == ref) {
                frames[j].age |= (1u << 31); 
                // - Sets MSB to record recent use
                found = true;
                break;
            }
        }

        if (!found) {
            pageFaults++;

            if ((int)frames.size() < numFrames) {
                // - If there's a frame available, page gets loaded
                Frame newFrame;
                newFrame.pageNumber = ref;
                newFrame.age = (1u << 31);
                frames.push_back(newFrame);
            }
            else {
                // - Finds the page with the smallest age (least recently used)
                int minIndex = 0;
                for (int j = 1; j < (int)frames.size(); j++) {
                    if (frames[j].age < frames[minIndex].age) {
                        minIndex = j;
                    }
                }
                // - Replaces it with the new page
                frames[minIndex].pageNumber = ref;
                frames[minIndex].age = (1u << 31);
            }
        }
    }

    return pageFaults;
}

// readReferences Function: reads a sequence of page references and stores them into a vector
vector<int> readReferences(const string& filename) {
    vector<int> references;
    ifstream file(filename);

    if (!file) {
        cout << "Error opening file.\n";
        return references;
    }

    int page;
    while (file >> page) {
        references.push_back(page);
    }

    file.close();
    return references;
}

// Main Loop
int main() {
    string filename;
    cout << "Enter reference file name: ";
    cin >> filename;

    vector<int> references = readReferences(filename);
    if (references.empty()) {
        cout << "No references loaded.\n";
        return 1;
    }

    int maxFrames;
    cout << "Enter maximum number of frames to test: ";
    cin >> maxFrames;

    cout << "\nFrames\tFaults per 1000 references\n";
    cout << "--------------------------------------\n";

    for (int frames = 1; frames <= maxFrames; frames++) {
        int faults = simulateAging(references, frames);
        double faultsPer1000 = (double)faults / references.size() * 1000;

        cout << frames << "\t"
             << fixed << setprecision(2)
             << faultsPer1000 << endl;
    }

    return 0;
}