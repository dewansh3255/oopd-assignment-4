#include <iostream>
#include "Student.h"

int main() {
    std::cout << "--- Testing Generic Student Class ---" << std::endl;

    // --- University A ---
    // Roll numbers are 'unsigned int', courses are 'string' acronyms, grades are 'double'
    using UniversityA_Student = Student<unsigned int, std::string, double>;

    UniversityA_Student s1(
        "Alice Smith", 
        2025001,  // unsigned int
        "Computer Science", 
        2025
    );
    s1.addPreviousCourse("OOPD", 9.5); // string, double
    s1.addPreviousCourse("DSA", 8.0);

    std::cout << "Student 1: " << s1.getName() 
              << ", Roll: " << s1.getRollNumber() << std::endl;


    // --- University B ---
    // Roll numbers are 'string', courses are 'int' codes, grades are 'char'
    using UniversityB_Student = Student<std::string, int, char>;

    UniversityB_Student s2(
        "Bob Johnson", 
        "MT-CS-2025-002", // string
        "Electronics", 
        2025
    );
    s2.addPreviousCourse(501, 'A');
    s2.addPreviousCourse(502, 'B');

    std::cout << "Student 2: " << s2.getName() 
              << ", Roll: " << s2.getRollNumber() << std::endl;

    return 0;
}