# OOPD Assignment 4: Generic University ERP

This project implements a generic, high-performance University ERP (Enterprise Resource Planning) system in C++. It is built to be highly flexible, supporting various data types for university records, and is optimized for common tasks like sorting and querying.

The solution demonstrates advanced C++17 features, including **templates**, **`std::variant`**, **multithreading** with `std::thread`, and **advanced data structures** (`std::map`).

## Features

* **Fully Generic `Student` Class**:
    The core `Student` class is built with C++ templates (`template <typename RollNoType, typename CourseIDType, typename GradeType>`). This allows the ERP to be configured for any university's data standards (e.g., `string` or `int` roll numbers) without changing the source code.

* **Mixed Course ID Support**:
    Accommodates the requirement to handle both IIIT-Delhi (`string` courses) and IIT-Delhi (`int` courses). This is achieved using `std::variant<string, int>`, a modern C++17 type-safe union, for the `CourseIDType`.

* **Parallel Merge Sort**:
    To handle bootstrapping the system with thousands of records, the program sorts students in parallel. It uses `std::thread` to sort two halves of the data concurrently and then merges them efficiently with `std::inplace_merge`.

* **Thread-Safe Logging**:
    A `std::mutex` is used to protect `std::cout` access. This prevents garbled output from the two sorting threads and ensures all logs (like thread completion times) are printed correctly.

* **Fast Grade Querying**:
    Implements a fast, non-linear search system for placement queries. An inverted index (`std::map`) is built on startup, allowing for `O(log N)` lookup of students by course and grade, completely avoiding a slow full-system scan.

* **Makefile Build System**:
    Includes a robust `Makefile` for easy compilation (`make`), cleaning (`make clean`), and linking of necessary libraries like `pthread` for `std::thread`.

## Design Decisions

This project's design prioritizes flexibility, performance, and modern C++ best practices.

1.  **Templates for Generics (Q1)**
    * **Why:** C++ templates were chosen over alternatives (like inheritance or void pointers) because they provide **compile-time polymorphism**. This is the most efficient, type-safe, and idiomatic C++ way to create a single, reusable class that can adapt to different data types for roll numbers and courses.

2.  **`std::variant` for Mixed Types (Q2)**
    * **Why:** To handle both `string` and `int` course codes, `std::variant` was the ideal choice. It's a type-safe, stack-allocated union that clearly expresses the "either-or" nature of the data, whereas other methods (like a custom struct) would be clunky and less efficient.

3.  **Multithreading for Sorting (Q3)**
    * **Why:** The requirement was to sort 3000 records using at least two threads.
    * **Approach:** The `std::vector` of students was split, and `std::thread` was used to sort each half independently. This is a simple and effective parallelization.
    * **Safety:** The sorting itself is inherently safe as the threads operate on *distinct* memory chunks. The only shared resource was `std::cout`, which was correctly protected by a `std::mutex` (`lock_guard`) to prevent race conditions in the log output.
    * **Merge:** `std::inplace_merge` was used post-join as it's the most efficient way to merge two already-sorted, contiguous ranges.

4.  **Inverted Index for Fast Lookups (Q5)**
    * **Why:** The requirement to "quickly" find students with ">9" in a course "without... a full linear search" pointed directly to an index.
    * **Structure:** A `map<MixedCourseID, map<int, vector<IIITD_Student*>>>` was used.
        * This structure gives `O(log C)` lookup for the course (outer map).
        * Then, it gives `O(log G)` lookup for the grade (inner map, using `lower_bound`).
        * Storing `IIITD_Student*` pointers instead of full objects is highly memory-efficient, avoiding thousands of object copies.
    * This design perfectly fulfills the requirement, turning a slow `O(N)` operation into a near-instant `O(log C + log G)` query.

## How to Build and Run

This project uses a `Makefile` and requires a C++17 compatible compiler (like `g++`) and the POSIX threads library.

1.  **Compile:**
    ```sh
    make
    ```
    This will use `g++` with the `-std=c++17` and `-pthread` flags to create the executable.

2.  **Run:**
    ```sh
    ./university_erp <csv_filename>
    ```
    * Example: `./university_erp students.csv`
    * If no filename is provided, it will default to looking for `students.csv`.

3.  **Clean Build Artifacts:**
    ```sh
    make clean
    ```
    This will remove the `university_erp` executable and all object files (`*.o`).