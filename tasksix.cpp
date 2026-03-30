#include <iostream>
#include <filesystem>
#include <vector>
#include <map>
#include <iomanip>
#include <string>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;

// getFileSizes Function
// 1. Starts from the given directory path
// 2. Recursively goes through all files and subdirectories
// 3. Stores the size of each regular file in a vector
vector<uintmax_t> getFileSizes(const fs::path& startPath) {
    vector<uintmax_t> fileSizes;
    try {

        if (!fs::exists(startPath)) {
            cerr << "Error: The given path does not exist." << endl;
            return fileSizes;
        }
        if (!fs::is_directory(startPath)) {
            cerr << "Error: The given path is not a directory." << endl;
            return fileSizes;
        }

        for (const auto& entry : fs::recursive_directory_iterator(startPath)) {
            try {
                if (fs::is_regular_file(entry.path())) {
                    fileSizes.push_back(fs::file_size(entry.path()));
                }
            } catch (const fs::filesystem_error&) {
                // Skip files that cannot be accessed
            }
        }
    } catch (const fs::filesystem_error& e) {
        cerr << "Filesystem error: " << e.what() << endl;
    }
    return fileSizes;
}

// buildHistogram Function: Groups the collected file sizes into bins based on the given bin width
map<uintmax_t, int> buildHistogram(const vector<uintmax_t>& fileSizes, uintmax_t binWidth) {
    map<uintmax_t, int> histogram;
    for (uintmax_t size : fileSizes) {
        uintmax_t binStart = (size / binWidth) * binWidth;
        histogram[binStart]++;
    }
    return histogram;
}

// printHistogram Function: Displays the histogram in the terminal using ranges and asterisks
void printHistogram(const map<uintmax_t, int>& histogram, uintmax_t binWidth) {
    cout << "\nFile Size Histogram\n";
    cout << "===================\n";
    for (const auto& bin : histogram) {
        uintmax_t start = bin.first;
        uintmax_t end = start + binWidth - 1;

        cout << setw(10) << start << " - "
             << setw(10) << end << " bytes : ";

        for (int i = 0; i < bin.second; i++) {
            cout << "*";
        }

        cout << " (" << bin.second << ")" << endl;
    }
}

// saveHistogramToCSV Function: Saves the histogram data into a CSV file so it can later be plotted
void saveHistogramToCSV(const map<uintmax_t, int>& histogram, uintmax_t binWidth) {
    ofstream file("histogram.csv");

    if (!file.is_open()) {
        cerr << "Error: Could not create histogram.csv file." << endl;
        return;
    }

    file << "bin_start,bin_end,count\n";

    for (const auto& bin : histogram) {
        uintmax_t start = bin.first;
        uintmax_t end = start + binWidth - 1;

        file << start << "," << end << "," << bin.second << "\n";
    }

    file.close();

    cout << "Histogram data saved to histogram.csv" << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <directory_path> <bin_width_in_bytes>" << endl;
        cerr << "Example: " << argv[0] << " /home/user/documents 1024" << endl;
        return 1;
    }

    string directoryPath = argv[1];

    uintmax_t binWidth = 0;
    try {
        binWidth = stoull(argv[2]);
    } catch (const invalid_argument&) {
        cerr << "Error: Bin width must be a valid integer." << endl;
        return 1;
    } catch (const out_of_range&) {
        cerr << "Error: Bin width value is out of range." << endl;
        return 1;
    }

    if (binWidth == 0) {
        cerr << "Error: Bin width must be greater than 0." << endl;
        return 1;
    }

    // Getting all file sizes from the given directory
    vector<uintmax_t> fileSizes = getFileSizes(directoryPath);

    if (fileSizes.empty()) {
        cout << "No files found, or the directory could not be read." << endl;
        return 0;
    }

    // Building the histogram from the collected file sizes
    map<uintmax_t, int> histogram = buildHistogram(fileSizes, binWidth);

    // Printing the total number of files and the histogram
    cout << "\nTotal files scanned: " << fileSizes.size() << endl;
    printHistogram(histogram, binWidth);

    // Saving the histogram data into a CSV file
    saveHistogramToCSV(histogram, binWidth);

    return 0;
}