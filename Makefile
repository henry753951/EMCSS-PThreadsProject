CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -pthread
LDFLAGS = -pthread

TARGET = main

SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
HEADERS = $(wildcard *.hpp)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

debug: CXXFLAGS += -g -DDEBUG
debug: clean $(TARGET)

.PHONY: all clean debug