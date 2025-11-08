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

// --- Type Aliases ---
using MixedCourseID = variant<string, int>;
using IIITD_Student = Student<string, MixedCourseID, int>;

// --- Global Mutex ---
mutex cout_mutex;

// --- Thread Worker Function (from Step 3) ---
void sort_chunk(vector<IIITD_Student>::iterator begin,
                vector<IIITD_Student>::iterator end,
                int thread_id) {
    
    auto start_time = chrono::high_resolution_clock::now();
    { 
        lock_guard<mutex> lock(cout_mutex);
        cout << "[Thread " << thread_id << "] Starting sort..." << endl;
    }

    std::sort(begin, end); 

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> diff_ms = end_time - start_time;

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "[Thread " << thread_id << "] Finished sort in " 
             << diff_ms.count() << " ms." << endl;
    }
}

// --- CSV Loading Function (from Step 3) ---
vector<IIITD_Student> loadStudentsFromCSV(const string& filename) {
    vector<IIITD_Student> students;
    ifstream file(filename);
    string line;

    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return students;
    }

    while (getline(file, line)) {
        stringstream ss(line);
        string name, roll, branch, year_str;
        getline(ss, name, ',');
        getline(ss, roll, ',');
        getline(ss, branch, ',');
        getline(ss, year_str, ',');
        students.emplace_back(name, roll, branch, stoi(year_str));
    }

    cout << "Loaded " << students.size() << " student records from CSV." << endl;
    return students;
}

// --- Print Function (NEW FOR STEP 4) ---
/**
 * @brief Prints the first 'count' students from a vector.
 * @param students A read-only reference to the student vector.
 * @param title A title to print for this section.
 * @param count The number of students to print.
 */
void printStudents(const vector<IIITD_Student>& students, const string& title, int count = 5) {
    cout << "\n--- " << title << " (First " << count << " Records) ---" << endl;
        for(int i = 0; i < count && i < students.size(); ++i) {
        cout << students[i].getRollNumber() << endl;
    }
}


// --- Main Function ---
int main() {
    cout << "--- Step 4: Displaying Records ---" << endl;

    // 1. Load 3000 records
    vector<IIITD_Student> students = loadStudentsFromCSV("students.csv");
    if (students.empty()) return 1;

    // 2. SHOW ORIGINAL ORDER
    // This call uses a const_iterator to read the data
    printStudents(students, "Original Order", 5); // <-- NEW

    // 3. Parallel Sorting (from Step 3)
    size_t mid_point = students.size() / 2;
    auto mid_iter = students.begin() + mid_point;

    cout << "\nStarting parallel sort with 2 threads..." << endl;
    
    // These calls use RandomAccessIterator (begin(), end()) to modify the data
    thread t1(sort_chunk, students.begin(), mid_iter, 1);
    thread t2(sort_chunk, mid_iter, students.end(), 2);
    t1.join();
    t2.join();
    cout << "\nBoth threads have finished." << endl;

    cout << "Merging the two sorted halves..." << endl;
    inplace_merge(students.begin(), mid_iter, students.end());
    cout << "Merge completed." << endl;

    // 4. SHOW SORTED ORDER
    // This call ALSO uses a const_iterator to read the data
    printStudents(students, "Sorted Order", 5); // <-- NEW

    return 0;
}