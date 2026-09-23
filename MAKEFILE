CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -Wall

SRC = $(wildcard src/*.cpp)
TESTS = $(wildcard tests/*.cpp)

SERVER_OBJ = $(SRC:.cpp=.o)
TEST_OBJ = $(TESTS:.cpp=.o)

SERVER = server
TEST = atomic_test

all: $(SERVER)

$(SERVER): $(SERVER_OBJ)
	$(CXX) $(SERVER_OBJ) -o $(SERVER)

$(TEST): $(TEST_OBJ)
	$(CXX) $(TEST_OBJ) -o $(TEST)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o tests/*.o $(SERVER) $(TEST)