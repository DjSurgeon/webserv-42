#!/bin/bash

# Configuration
SERVER_PORT=8080
CLIENT_COUNT=200
GREEN="\033[32m"
RED="\033[31m"
YELLOW="\033[33m"
RESET="\033[0m"

echo -e "${YELLOW}=== STARTING MASSIVE STRESS TEST BATTERY ===${RESET}\n"

# 1. Build everything
echo "Compiling server and stress client..."
make all stress > /dev/null
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed!${RESET}"
    exit 1
fi

# 2. Start server
./webserv > stress_server.log 2>&1 &
SERVER_PID=$!
sleep 1

if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo -e "${RED}Server failed to start!${RESET}"
    cat stress_server.log
    exit 1
fi

echo -e "Server started (PID: $SERVER_PID). Target: localhost:$SERVER_PORT\n"

# 3. Helper for running modes
run_stress_mode() {
    local mode=$1
    local count=$2
    echo -e "${YELLOW}[Mode: $mode]${RESET} Launching $count connections..."
    ./bin/stress_client --mode "$mode" --count "$count" --port "$SERVER_PORT"
    
    if ! kill -0 $SERVER_PID 2>/dev/null; then
        echo -e "${RED}CRASH DETECTED! Server died during $mode mode.${RESET}"
        cat stress_server.log
        exit 1
    fi
    echo -e "${GREEN}✓ Server survived $mode mode.${RESET}\n"
}

# 4. Execute modes
run_stress_mode "flood" 300
run_stress_mode "garbage" 200
run_stress_mode "drop" 300
run_stress_mode "slowloris" 100
run_stress_mode "delete" 200

# 5. Final verification with a normal request
echo "Verifying server responsiveness after stress..."
curl -s -o /dev/null http://localhost:$SERVER_PORT
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Server is still responsive!${RESET}"
else
    echo -e "${RED}Server is unresponsive after stress!${RESET}"
    exit 1
fi

# 6. Cleanup
echo -e "\nCleaning up..."
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
rm stress_server.log

echo -e "${GREEN}=== ALL STRESS TESTS PASSED SUCCESSFULLY ===${RESET}"
exit 0
