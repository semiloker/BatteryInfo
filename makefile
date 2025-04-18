TARGET = bin\battery.exe
CC = g++
CFLAGS = -Wall -lgdi32 -lshell32 -mwindows -static -static-libgcc -static-libstdc++ -lwinpthread
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

LIBS = -lsetupapi -lole32 -loleaut32 -ld2d1 -ldwrite -ldwmapi -lpdh	

$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(OBJ_DIR):
	md $(OBJ_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

clean:
	del /Q "$(OBJ_DIR)\*.o" "$(TARGET)"
	rmdir /S /Q "$(OBJ_DIR)" "$(BIN_DIR)"

.PHONY: clean
