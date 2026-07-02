CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

TARGET = shell
SRC = src/main.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)