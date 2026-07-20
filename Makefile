CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

TARGET = nut-shell
SRC = src/main.cpp src/parser.cpp src/executor.cpp src/helper/handleBuiltins.cpp

TEST_TARGET_1 = test_parser
TEST_SRC_1 = test/test_parser.cpp src/parser.cpp

TEST_TARGET_2 = test_executor
TEST_SRC_2 = test/test_executor.cpp src/executor.cpp src/parser.cpp src/helper/handleBuiltins.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

$(TEST_TARGET_1): $(TEST_SRC_1)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET_1) $(TEST_SRC_1)

$(TEST_TARGET_2): $(TEST_SRC_2)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET_2) $(TEST_SRC_2) -lreadline

clean:
	rm -f $(TARGET) $(TEST_TARGET_1) $(TEST_TARGET_2)