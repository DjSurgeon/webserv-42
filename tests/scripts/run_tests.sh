#!/bin/bash

# Configuration
CXX=c++
CXXFLAGS="-Wall -Wextra -Werror -std=c++98"
INC="-Isrc"
SRC="src/network/ListeningSocket.cpp src/network/ClientSocket.cpp"
TEST_SRC="tests/network/test_sockets.cpp"
BIN="bin/test_sockets"

# Create bin directory if it doesn't exist
mkdir -p bin

echo "🔨 Compiling tests..."
$CXX $CXXFLAGS $INC $SRC $TEST_SRC -o $BIN

if [ $? -ne 0 ]; then
    echo "❌ Compilation failed!"
    exit 1
fi

echo "✅ Compilation successful. Running tests with Valgrind..."
valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./$BIN

if [ $? -ne 0 ]; then
    echo "❌ Tests failed or Valgrind reported errors!"
    exit 1
fi

echo "🎉 All tests passed successfully!"
