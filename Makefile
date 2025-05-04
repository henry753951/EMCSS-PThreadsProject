# 編譯器和標誌
CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -pthread
LDFLAGS = -pthread
INCLUDE = -I include

# 目錄設置
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
TARGET = program

# 自動尋找源檔案和頭檔案
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
HEADERS = $(wildcard $(INCLUDE_DIR)/*.hpp)

# 預設目標
all: $(BUILD_DIR) $(TARGET)

# 創建建置目錄
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 連結目標
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

# 編譯源檔案
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

# 清理
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# 除錯模式
debug: CXXFLAGS += -g -DDEBUG
debug: clean all

# 聲明假目標
.PHONY: all clean debug