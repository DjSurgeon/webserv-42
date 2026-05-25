#!/bin/bash

# Configuration
SERVER_PORT=8080
NUM_CLIENTS=20
DURATION=2

echo "Starting system-level concurrency test..."

# Start the server in the background
./webserv > server.log 2>&1 &
SERVER_PID=$!

# Wait for server to start
sleep 1

if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "Error: Server failed to start."
    cat server.log
    exit 1
fi

echo "Server is running (PID: $SERVER_PID). Launching $NUM_CLIENTS concurrent clients..."

# Function to simulate a client
simulate_client() {
    local id=$1
    local response
    # Connect and send a fragmented request
    response=$( (
        echo -n "GET /index.html"
        sleep 0.2
        echo -n " HTTP/1.1"
        sleep 0.2
        echo -e "\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n"
    ) | nc 127.0.0.1 $SERVER_PORT 2>/dev/null )
    
    if [[ "$response" == *"HTTP/1.1"* ]]; then
        return 0
    else
        return 1
    fi
}

# Launch clients in background and store their PIDs
CLIENT_PIDS=()
for i in $(seq 1 $NUM_CLIENTS); do
    simulate_client $i &
    CLIENT_PIDS+=($!)
done

# Wait for all clients to finish and check their exit status
FAILED_CLIENTS=0
for pid in "${CLIENT_PIDS[@]}"; do
    if ! wait $pid; then
        ((FAILED_CLIENTS++))
    fi
done

if [ $FAILED_CLIENTS -eq 0 ]; then
    echo "All $NUM_CLIENTS clients received correct responses."
    SUCCESS=0
else
    echo "$FAILED_CLIENTS clients failed to receive correct responses."
    SUCCESS=1
fi

# Cleanup
echo "Cleaning up..."
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

if [ $SUCCESS -eq 0 ]; then
    echo "✅ Concurrency test PASSED!"
else
    echo "❌ Concurrency test FAILED!"
fi

exit $SUCCESS
