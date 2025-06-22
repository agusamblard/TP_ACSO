# Compiler settings
CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread -g

# Default target (main)
TARGET = threadpool
SRC = thread-pool.cc Semaphore.cc main.cc

# Build target with main.cc
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build target with tptest.cc
tptest: thread-pool.cc Semaphore.cc tptest.cc
	$(CXX) $(CXXFLAGS) -o tptest thread-pool.cc Semaphore.cc tptest.cc

# Build target with tpcustomtest.cc
tpcustomtest: thread-pool.cc Semaphore.cc tpcustomtest.cc
	$(CXX) $(CXXFLAGS) -o tpcustomtest thread-pool.cc Semaphore.cc tpcustomtest.cc

# Clean up build artifacts
clean:
	rm -f $(TARGET) tptest tpcustomtest $(OBJ)

.PHONY: all clean tptest tpcustomtest
