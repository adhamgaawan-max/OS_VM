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
