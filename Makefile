# Compiler
CXX = g++

# Compiler flags
# -std=c++17: Use C++17 standard
# -pthread:   Link the POSIX threads library (for std::thread)
# -Wall:      Enable all warnings
# -g:         Include debugging symbols
CXXFLAGS = -std=c++17 -pthread -Wall -g

# The final executable name
TARGET = university_erp

# All source files
SRCS = main.cpp # We can add more files here later (e.g., Student.cpp)

# All object files, derived from source files
OBJS = $(SRCS:.cpp=.o)

# Default rule: build the target
all: $(TARGET)

# Rule to link the target executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile a .cpp file into a .o file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean