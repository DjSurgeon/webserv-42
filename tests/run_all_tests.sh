#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Define colors
GREEN="\033[32m"
RED="\033[31m"
RESET="\033[0m"

echo -e "=== COMPILING AND RUNNING ALL TESTS ===\n"

# Ensure bin directory exists
mkdir -p bin

# Helper function to compile and run a test
run_test() {
    local test_name=$1
    local sources=$2
    local test_file=$3

    echo "Compiling ${test_name}..."
    c++ -Wall -Wextra -Werror -std=c++98 -Isrc ${sources} ${test_file} -o bin/${test_name}

    echo "Running ${test_name}..."
    ./bin/${test_name}
    echo -e "${GREEN}✓ ${test_name} passed successfully!${RESET}\n"
}

# Run each test suite
run_test "test_context" "src/config/Context.cpp" "tests/test_context.cpp"
run_test "test_location_config" "src/config/Context.cpp src/config/LocationConfig.cpp" "tests/test_location_config.cpp"
run_test "test_server_config" "src/config/Context.cpp src/config/LocationConfig.cpp src/config/ServerConfig.cpp" "tests/test_server_config.cpp"
run_test "test_http_request" "src/http/HttpRequest.cpp" "tests/test_http_request.cpp"
run_test "test_http_response" "src/http/HttpResponse.cpp" "tests/test_http_response.cpp"
run_test "test_parser_skeleton" "src/http/HttpRequest.cpp src/http/RequestParser.cpp" "tests/test_parser_skeleton.cpp"
run_test "test_parser_method" "src/http/HttpRequest.cpp src/http/RequestParser.cpp" "tests/test_parser_method.cpp"
run_test "test_parser_uri" "src/http/HttpRequest.cpp src/http/RequestParser.cpp" "tests/test_parser_uri.cpp"
run_test "test_parser_version" "src/http/HttpRequest.cpp src/http/RequestParser.cpp" "tests/test_parser_version.cpp"
run_test "test_parser_header_key" "src/http/HttpRequest.cpp src/http/RequestParser.cpp" "tests/test_parser_header_key.cpp"
run_test "test_parser_header_value" "src/http/HttpRequest.cpp src/http/RequestParser.cpp" "tests/test_parser_header_value.cpp"
run_test "test_parser_body" "src/http/HttpRequest.cpp src/http/RequestParser.cpp" "tests/test_parser_body.cpp"
run_test "test_parser_stress" "src/network/ClientSocket.cpp src/http/HttpRequest.cpp src/http/RequestParser.cpp src/network/EventLoop.cpp tests/test_globals.cpp" "tests/test_parser_stress.cpp"
run_test "test_integration" "src/network/ClientSocket.cpp src/http/HttpRequest.cpp src/http/RequestParser.cpp src/network/EventLoop.cpp tests/test_globals.cpp" "tests/test_integration.cpp"

echo -e "=== RUNNING SHELL SCRIPT TESTS ===\n"
echo "Running test_concurrency.sh..."
./tests/test_concurrency.sh

echo -e "\n${GREEN}All test suites compiled and executed successfully!${RESET}"
