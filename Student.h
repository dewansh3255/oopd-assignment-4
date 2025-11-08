#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include <vector>
#include <map>

// We declare three template "typename" parameters.
// This lets us specify these types when we create a Student object.
template <typename RollNoType, typename CourseIDType, typename GradeType>
class Student {

// PRIVATE: These variables are hidden and protected
private:
    std::string name;
    RollNoType rollNumber;
    std::string branch;
    int startingYear;

    // A list of courses the student is currently taking
    std::vector<CourseIDType> currentCourses;
    
    // A map to store previous courses and their grades
    std::map<CourseIDType, GradeType> previousCourses;

// PUBLIC: These are the methods used to interact with the object
public:
    // --- Constructor ---
    // Initializes a new Student object
    Student(const std::string& n, const RollNoType& roll, const std::string& br, int year)
        : name(n), rollNumber(roll), branch(br), startingYear(year) {
    }

    // --- Mutator Methods (Setters) ---

    // Adds a course to the "current courses" list
    void addCurrentCourse(const CourseIDType& course) {
        currentCourses.push_back(course);
    }

    // Adds a completed course and its grade
    void addPreviousCourse(const CourseIDType& course, const GradeType& grade) {
        previousCourses[course] = grade;
    }

    // --- Accessor Methods (Getters) ---
    
    const std::string& getName() const {
        return name;
    }

    const RollNoType& getRollNumber() const {
        return rollNumber;
    }


    const std::map<CourseIDType, GradeType>& getPreviousCourses() const {
        return previousCourses;
    }
    
    // --- Operator Overloading ---
    bool operator<(const Student& other) const {
        return rollNumber < other.rollNumber;
    }
};

#endif // STUDENT_H