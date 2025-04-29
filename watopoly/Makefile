CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pedantic -fmodules-ts

MODULES = Declarations.o Implementations.o

TARGET = watopoly

all: $(TARGET)

$(TARGET): $(MODULES) harness.o
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $^

# Module dependencies
Declarations.o: Declarations.cc

Implementations.o: Implementations.cc Declarations.o

harness.o: harness.cc Declarations.o Implementations.o

# Run the program
run: $(TARGET)
	./$(TARGET)

# Run with testing mode
test: $(TARGET)
	./$(TARGET) -testing

# Run with a save file
load: $(TARGET)
	@echo "Enter the save file name: "
	@read file; ./$(TARGET) -load $$file

clean:
	rm -f $(TARGET) *.o *.gcm
	rm -rf gcm.cache

.PHONY: all clean run test load