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

        // Create the student object
        students.emplace_back(name, roll, branch, stoi(year_str));

        // --- ADDING DUMMY GRADES ---
        // Get a reference to the student we just added
        IIITD_Student& newStudent = students.back();
        
        // Give every student a grade for "OOPD" from 6 to 10
        newStudent.addPreviousCourse("OOPD", (students.size() % 5) + 6);
        
        // Give some students a grade for the IIT-D course '701'
        if (students.size() % 3 == 0) {
            newStudent.addPreviousCourse(701, ((students.size() / 3) % 3) + 8);
        }
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

// --- Type Alias for our Grade Index (NEW FOR STEP 5) ---
using GradeIndex = map<MixedCourseID, map<int, vector<IIITD_Student*>>>;

/**
 * @brief Builds the inverted grade index from the vector of students.
 * @param students A vector of all student objects.
 * @return The fully populated GradeIndex map.
 */
GradeIndex buildGradeIndex(vector<IIITD_Student>& students) {
    cout << "\nBuilding fast-lookup grade index..." << endl;
    GradeIndex index;

    // Iterate through all 3000 students ONCE
    for (IIITD_Student& student : students) {
        
        // Get the map of previous courses for this student
        // Note: getPreviousCourses() was added in Student.h in Step 1
        for (const auto& pair : student.getPreviousCourses()) {
            const MixedCourseID& course = pair.first;
            const int& grade = pair.second;
            
            // Add a POINTER to this student into the correct index bucket
            index[course][grade].push_back(&student);
        }
    }
    cout << "Index build complete." << endl;
    return index;
}

/**
 * @brief Finds students using the index, avoiding a linear search.
 * @param index The inverted grade index.
 * @param course The course to query.
 * @param minGrade The minimum grade required.
 */
void findStudentsByGrade(const GradeIndex& index, const MixedCourseID& course, int minGrade) {
    cout << "\n--- Query: Students with >= " << minGrade << " in course ";
    
    // Helper to print the variant course ID
    // We use std::visit to handle the string or int
    visit([](const auto& value){ cout << "'" << value << "'"; }, course);
    cout << " ---" << endl;

    // 1. Find the course (Fast O(log C) lookup)
    auto courseIt = index.find(course);
    if (courseIt == index.end()) {
        cout << "No students found for this course." << endl;
        return;
    }

    // 'courseIt->second' is the map<int, vector<Student*>>
    const auto& gradeMap = courseIt->second;

    // 2. Find the first grade >= minGrade (Fast O(log G) lookup)
    // We use lower_bound() to find the first element that is NOT LESS than minGrade
    auto gradeIt = gradeMap.lower_bound(minGrade);

    // 3. Iterate from that point to the end
    int count = 0;
    for (; gradeIt != gradeMap.end(); ++gradeIt) {
        // 'gradeIt->first' is the grade (e.g., 9, 10)
        // 'gradeIt->second' is the vector<Student*>
        for (IIITD_Student* s : gradeIt->second) {
            cout << "Found: " << s->getName() << " (" << s->getRollNumber() 
                 << ") with grade " << gradeIt->first << endl;
            count++;
        }
    }

    if (count == 0) {
        cout << "No students met the grade requirement." << endl;
    }
}

// --- Main Function ---
int main() {
    cout << "--- OOPD Assignment 4 ---" << endl;

    vector<IIITD_Student> students = loadStudentsFromCSV("students.csv");
    if (students.empty()) return 1;

    // --- Q4: Show Original Order ---
    printStudents(students, "Original Order", 5);

    // --- Q3: Parallel Sorting ---
    size_t mid_point = students.size() / 2;
    auto mid_iter = students.begin() + mid_point;
    cout << "\nStarting parallel sort with 2 threads..." << endl;
    thread t1(sort_chunk, students.begin(), mid_iter, 1);
    thread t2(sort_chunk, mid_iter, students.end(), 2);
    t1.join();
    t2.join();
    cout << "\nBoth threads have finished." << endl;
    cout << "Merging the two sorted halves..." << endl;
    inplace_merge(students.begin(), mid_iter, students.end());
    cout << "Merge completed." << endl;

    // --- Q4: Show Sorted Order ---
    printStudents(students, "Sorted Order", 5);

    // --- Q5: Build and Query Index ---
    
    // 1. Build the index
    GradeIndex index = buildGradeIndex(students);

    // 2. Run queries
    // Query 1: Find students with >= 9 in "OOPD"
    findStudentsByGrade(index, "OOPD", 9);
    
    // Query 2: Find students with >= 9 in the IIT-D course '701'
    findStudentsByGrade(index, 701, 9);
    
    // Query 3: A query that should find nothing
    findStudentsByGrade(index, "DSA", 9);

    return 0;
}