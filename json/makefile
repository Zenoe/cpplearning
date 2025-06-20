#CXX = g++
# CXXFLAGS = -std=c++17 -I../thirdparty -I../ -Wall -Wextra -Wpedantic -Wshadow -Werror -O2 -g
# CXXFLAGS = -std=c++17 -I../thirdparty -I../ -Wall -Wextra -Wpedantic -Wshadow -O2 -g
# TARGET = jsontest

# all: $(TARGET)

# $(TARGET): main.cpp
# 	$(CXX) $(CXXFLAGS) $< -o $@

# clean:
# 	rm -f $(TARGET)

# compile_commands.json:
# 	@echo '[{"directory": "$(shell pwd)", "command": "$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET)", "file": "main.cpp"}]' > $@

# with pch
# CXX = g++
# CXXFLAGS = -std=c++17 -I../thirdparty -I../ -Wall -Wextra -Wpedantic -Wshadow
# PCH_HEADER = stdafx.h
# PCH = $(PCH_HEADER).gch
# TARGET = jsontest

# # Rule to precompile the header
# $(PCH): $(PCH_HEADER)
# 	$(CXX) $(CXXFLAGS) -xc++-header $< -o $@

# # Rule to compile main.cpp (depends on PCH)
# main.o: main.cpp $(PCH)
# 	$(CXX) $(CXXFLAGS) -include $(PCH_HEADER) -c $< -o $@

# # Link
# $(TARGET): main.o
# 	$(CXX) $^ -o $@

# clean:
# 	rm -f $(TARGET) *.o $(PCH)


CXX = g++
CXXFLAGS = -std=c++17 -I../thirdparty -I../ -Wall -Wextra -Wpedantic -Wshadow
PCH_HEADER = stdafx.h
PCH = $(PCH_HEADER).gch
TARGET = jsontest
OBJECTS = main.o

.PHONY: all clean

# Default target
all: $(TARGET)

# Rule to precompile the header
$(PCH): $(PCH_HEADER)
	$(CXX) $(CXXFLAGS) -xc++-header $< -o $@

# Compile source file with PCH
main.o: main.cpp $(PCH)
	$(CXX) $(CXXFLAGS) -include $(PCH_HEADER) -c $< -o $@

# Link final executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET)

# Clean up
clean:
	rm -f $(TARGET) *.o $(PCH)
