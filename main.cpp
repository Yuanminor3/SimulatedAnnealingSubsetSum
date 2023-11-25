#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <random>
#include <chrono>
#include "SimAnneal.h"
#include <unordered_set>
#include <iomanip>

using namespace std;

FILE *_kiaDebugFile; // Global debug file pointer

bool readInputFile(const string& filename, vector<int>& elements, int& target, int& numberOfElements) {
    ifstream file(filename);
    if (!file) {
        cerr << "Cannot open file " << filename << endl;
        return false;
    }

    file >> numberOfElements; // First line is the number of elements
    file >> target; // Second line is the target sum
    int element;
    while (file >> element) {
        elements.push_back(element);
    }

    return true;
}


int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        cerr << "Usage: " << argv[0] << " <input file> [target sum override]" << endl;
        return -1;
    }
    // Start the clock
    clock_t start = clock();

    string inputFileName = argv[1];
    numberOfElements = 0;
    targetSum = 0;

    // Initialize the debug log file
    _kiaDebugFile = fopen("anneal.log", "w");
    // error handling
    if (_kiaDebugFile == NULL) {
        cout << "Cannot create file anneal.log." << endl;
        return -1;
    }

    // Read input file
    if (!readInputFile(inputFileName, elements, targetSum, numberOfElements)) {
        // error handling
        cerr << "Error reading input file." << endl;
        fclose(_kiaDebugFile);
        return -1;
    }

    // Override target sum if provided as a second argument
    if (argc == 3) {
        targetSum = atoi(argv[2]);
    }

    CSimAnneal subset(numberOfElements);
    subset.doAnnealing();

    // output
    cout << endl;
    
    // Stop the clock
    clock_t end = clock();
    // Calculate the CPU time used by the program
    double cpu_time_used = static_cast<double>(end - start) / CLOCKS_PER_SEC;
    cout << fixed << setprecision(3);
    cout << "CPU time used: " << cpu_time_used << " seconds" << endl;
    // Output target sum and final cost
    cout << "The target sum: " << targetSum << endl;
    cout << "The final cost (0 means it did find the exact sum): " << finalCost << endl;
    cout << "Subsize = " << currentSolution.size() << endl;


    							// ************* Outputs a txt file *******************

    ofstream file("output.txt");

    file << fixed << setprecision(3);
    file << "CPU time used: " << cpu_time_used << " seconds" << endl;
    file << "The target sum: " << targetSum << endl;
    file << "The final cost (0 means it did find the exact sum): " << finalCost << endl;
    file << "Subsize = " << currentSolution.size() << endl;
    file << endl;

    long s=0;
    cout << endl;

    for (int index : currentSolution) {
        cout << index << "\t" << elements[index] << endl;
        file << index << "\t" << elements[index] << endl;
        s += elements[index];
    }
    cout << endl;
    file << endl;

    cout << "Sum of Subset = " << s << endl;
    file << "Sum of Subset = " << s << endl;

    unordered_set<int> seenIndexes;
    bool hasDuplicate = false;

    for (int index : currentSolution) {
        if (seenIndexes.find(index) != seenIndexes.end()) {
            cout << "Duplicate index found: " << index << endl;
            file << "Duplicate index found: " << index << endl;
            hasDuplicate = true;
            break;
        }
        seenIndexes.insert(index);
    }

    if (!hasDuplicate) {
        cout << "No duplicates found." << endl;
        file << "No duplicates found." << endl;
    }

    // Close the debug log file before the program exits
    fflush(_kiaDebugFile);
    fclose(_kiaDebugFile);
    file.close(); // Close the file

    return 0;
}


