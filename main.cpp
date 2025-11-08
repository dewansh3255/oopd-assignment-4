#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <map>
#include "Student.h"
using namespace std;
using MixedCourseID = variant<string, int>;
using IIITD_Student = Student<string, MixedCourseID, int>;
int main() {
    cout << "--- Step 2: Testing IIIT-D and IIT-D Courses ---" << endl;
    IIITD_Student s1("Dewansh Khandelwal", "MT25067", "CSE", 2025);
    // 1. Add an IIIT-D course (using a string)
    s1.addPreviousCourse("OOPD", 9);
    // 2. Add an IIT-D course (using an int)
    s1.addPreviousCourse(701, 10);
    cout << "Successfully created student: " << s1.getName() << endl;
    cout << "Added both string ('OOPD') and int (701) courses." << endl;
    return 0;
}