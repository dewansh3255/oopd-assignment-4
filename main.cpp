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
using GradeIndex = map<MixedCourseID, map<int, vector<IIITD_Student*>>>;

// --- Global Mutex ---
mutex cout_mutex;

// --- Thread Worker Function (Sorting Pointers) ---
// UPDATE: Now sorts pointers (IIITD_Student*) instead of full objects
void sort_chunk(vector<IIITD_Student*>::iterator begin,
                vector<IIITD_Student*>::iterator end,
                int thread_id) {
    
    auto start_time = chrono::high_resolution_clock::now();
    { 
        lock_guard<mutex> lock(cout_mutex);
        cout << "  [Thread " << thread_id << "] Processing chunk..." << endl;
    }

    // Sort the pointers by dereferencing them to compare the actual Student objects
    sort(begin, end, [](const IIITD_Student* a, const IIITD_Student* b) {
        return *a < *b; // Uses the operator< defined in Student.h
    });

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> diff_ms = end_time - start_time;

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "  [Thread " << thread_id << "] Done in " << diff_ms.count() << " ms." << endl;
    }
}

// --- Helper Functions ---

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

        if(name.empty() || roll.empty()) continue;

        int year = 0;
        try { year = stoi(year_str); } catch (...) { continue; }

        students.emplace_back(name, roll, branch, year);

        // --- ADDING DUMMY GRADES ---
        IIITD_Student& newStudent = students.back();
        newStudent.addPreviousCourse("OOPD", (rand() % 5) + 6);
        
        if (students.size() % 3 == 0) {
            newStudent.addPreviousCourse(701, ((students.size() / 3) % 3) + 8);
        }
    }
    cout << "Successfully loaded " << students.size() << " records." << endl;
    return students;
}

// UPDATE: Supports printing direct objects OR pointers
template <typename T>
void printStudents(const vector<T>& container, const string& title, int count = 5) {
    cout << "\n--- " << title << " (Showing first " << count << ") ---" << endl;
    if(container.empty()) return;
    
    int limit = min((int)container.size(), count);
    for(int i = 0; i < limit; ++i) {
        // Handle both Student and Student* types
        if constexpr (is_pointer<T>::value)
            cout << *(container[i]) << endl;
        else
            cout << container[i] << endl;
    }
}

GradeIndex buildGradeIndex(vector<IIITD_Student>& students) {
    cout << "\nBuilding fast-lookup grade index..." << endl;
    GradeIndex index;
    // Iterate by reference to avoid copying
    for (IIITD_Student& student : students) {
        for (const auto& pair : student.getPreviousCourses()) {
            index[pair.first][pair.second].push_back(&student);
        }
    }
    cout << "Index build complete." << endl;
    return index;
}

void findStudentsByGrade(const GradeIndex& index, const MixedCourseID& course, int minGrade) {
    cout << "\n--- Query: Students with >= " << minGrade << " in ";
    visit([](const auto& value){ cout << "'" << value << "'"; }, course);
    cout << " ---" << endl;
    
    auto courseIt = index.find(course);
    if (courseIt == index.end()) {
        cout << "No students found for this course." << endl;
        return;
    }

    const auto& gradeMap = courseIt->second;
    auto gradeIt = gradeMap.lower_bound(minGrade);

    int count = 0;
    for (; gradeIt != gradeMap.end(); ++gradeIt) {
        for (IIITD_Student* s : gradeIt->second) {
            cout << "Found: " << *s << " with grade " << gradeIt->first << endl;
            count++;
        }
    }

    if (count == 0) cout << "No students met the grade requirement." << endl;
}

// --- Main Function ---
int main(int argc, char* argv[]) {
    cout << "--- OOPD Assignment 4 ---" << endl;
    srand(time(0)); // Seed random for grade generation

    string csv_filename = "students.csv"; 
    if (argc > 1) csv_filename = argv[1];

    // 1. Load Data (Insertion Order)
    vector<IIITD_Student> students = loadStudentsFromCSV(csv_filename);
    if (students.empty()) return 1;

    // 2. Create a "View" of Pointers (for sorting without copying)
    // This satisfies "without copying the whole data"
    vector<IIITD_Student*> sorted_view;
    sorted_view.reserve(students.size());
    for (auto& s : students) sorted_view.push_back(&s);

    // 3. Show Original Order (Iterating vector<Student>)
    printStudents(students, "Original Insertion Order", 5);

    // 4. Parallel Sort on Pointers
    cout << "\nStarting parallel sort (on pointers) with 2 threads..." << endl;
    size_t mid = sorted_view.size() / 2;
    
    thread t1(sort_chunk, sorted_view.begin(), sorted_view.begin() + mid, 1);
    thread t2(sort_chunk, sorted_view.begin() + mid, sorted_view.end(), 2);
    t1.join();
    t2.join();
    
    cout << "Merging the two sorted halves..." << endl;
    // Merge logic for pointers
    inplace_merge(sorted_view.begin(), sorted_view.begin() + mid, sorted_view.end(), 
        [](const IIITD_Student* a, const IIITD_Student* b) { return *a < *b; });
    cout << "Merge completed." << endl;

    // 5. Show Sorted Order (Iterating vector<Student*>)
    printStudents(sorted_view, "Sorted View (via Pointers)", 5);

    // 6. Index & Search
    GradeIndex index = buildGradeIndex(students);
    findStudentsByGrade(index, "OOPD", 9);
    findStudentsByGrade(index, 701, 9);
    findStudentsByGrade(index, "DSA", 9);

    return 0;
}