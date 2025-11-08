#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>
#include "Student.h"

using namespace std;

// --- Type Aliases (from Step 2) ---
using MixedCourseID = variant<string, int>;
using IIITD_Student = Student<string, MixedCourseID, int>;

// --- Global Mutex for Thread-Safe Logging (Q3) ---
// This prevents threads from writing to cout at the same time
mutex cout_mutex;

/**
 * @brief The worker function for each thread to sort a chunk of the vector.
 * @param begin Iterator to the start of the chunk
 * @param end Iterator to the end of the chunk
 * @param thread_id An ID for logging
 */
void sort_chunk(vector<IIITD_Student>::iterator begin,
                vector<IIITD_Student>::iterator end,
                int thread_id) {
    
    // Log start time
    auto start_time = chrono::high_resolution_clock::now();

    { // Use lock_guard in a block to limit its scope
        lock_guard<mutex> lock(cout_mutex);
        cout << "[Thread " << thread_id << "] Starting sort..." << endl;
    }

    // --- The actual sorting ---
    std::sort(begin, end); // std::sort uses the operator< we defined in Student.h

    // Log end time
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> diff_ms = end_time - start_time;

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "[Thread " << thread_id << "] Finished sort in " 
             << diff_ms.count() << " ms." << endl;
    }
}

/**
 * @brief Loads student records from a CSV file.
 * @param filename The path to the students.csv file
 * @return A vector of IIITD_Student objects
 */
vector<IIITD_Student> loadStudentsFromCSV(const string& filename) {
    vector<IIITD_Student> students;
    ifstream file(filename);
    string line;

    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return students;
    }

    // Read the file line by line
    while (getline(file, line)) {
        stringstream ss(line);
        string name, roll, branch, year_str;

        // Split the line by commas
        getline(ss, name, ',');
        getline(ss, roll, ',');
        getline(ss, branch, ',');
        getline(ss, year_str, ',');

        // Create the student object and add it to the vector
        students.emplace_back(name, roll, branch, stoi(year_str));
    }

    cout << "Loaded " << students.size() << " student records from CSV." << endl;
    return students;
}


int main() {
    cout << "--- Step 3: CSV Loading & Parallel Sorting ---" << endl;

    // 1. Load 3000 records from the CSV file
    vector<IIITD_Student> students = loadStudentsFromCSV("students.csv");

    if (students.empty()) {
        return 1; // Exit if loading failed
    }

    // --- Parallel Sorting ---

    // Find the midpoint to split the vector
    size_t mid_point = students.size() / 2;
    auto mid_iter = students.begin() + mid_point;

    cout << "\nStarting parallel sort with 2 threads..." << endl;

    // 2. Launch two threads
    thread t1(sort_chunk, students.begin(), mid_iter, 1);
    thread t2(sort_chunk, mid_iter, students.end(), 2);

    // 3. Wait for both threads to finish their work
    t1.join();
    t2.join();

    cout << "\nBoth threads have finished." << endl;

    // 4. Merge the two sorted halves
    // This part runs in the main thread
    cout << "Merging the two sorted halves..." << endl;
    auto merge_start = chrono::high_resolution_clock::now();
    
    inplace_merge(students.begin(), mid_iter, students.end());
    
    auto merge_end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> merge_ms = merge_end - merge_start;
    cout << "Merge completed in " << merge_ms.count() << " ms." << endl;

    // (We'll add the printing for Q4 in the next step)
    cout << "\n--- Verification (First 5 Students after sort) ---" << endl;
    for (int i = 0; i < 5; ++i) {
        cout << students[i].getRollNumber() << endl;
    }

    return 0;
}