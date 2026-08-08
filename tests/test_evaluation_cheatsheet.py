#!/usr/bin/env python3
import socket
import sys
import os
import time

HOST = '127.0.0.1'
PORT = 8080

def send_request(req_str, port=PORT):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(2.0)
    try:
        s.connect((HOST, port))
        s.sendall(req_str.encode('utf-8'))
        response = b""
        while True:
            chunk = s.recv(4096)
            if not chunk:
                break
            response += chunk
        return response
    except Exception as e:
        return b"Error: " + str(e).encode('utf-8')
    finally:
        s.close()

def send_raw_request(req_bytes, port=PORT):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(2.0)
    try:
        s.connect((HOST, port))
        s.sendall(req_bytes)
        response = b""
        while True:
            chunk = s.recv(4096)
            if not chunk:
                break
            response += chunk
        return response
    except Exception as e:
        return b"Error: " + str(e).encode('utf-8')
    finally:
        s.close()

print("🚀 Starting Final Evaluation Cheatsheet Tests...\n")
failed = 0

# ---------------------------------------------------------
# Test 2: Virtual Hosts
# ---------------------------------------------------------
print("--- 2. Virtual Hosts ---")
res = send_request("GET / HTTP/1.1\r\nHost: unknown_host\r\nConnection: close\r\n\r\n")
status = res.split(b'\r\n')[0]
if b'200' in status:
    print("✅ PASS: Fallback to default host works.")
else:
    print(f"❌ FAIL: Fallback host returned {status}")
    failed += 1

# ---------------------------------------------------------
# Test 3: Error Codes & Body Limits
# ---------------------------------------------------------
print("\n--- 3. Error Codes & Limits ---")
res = send_request("GET /archivo_que_no_existe_nunca.html HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
if b'404' in res:
    print("✅ PASS: 404 Custom Not Found works.")
else:
    print("❌ FAIL: Did not return 404 for missing file.")
    failed += 1

# Note: Testing 413 requires a location with a small max_body_size.
# In raul.conf, global client_max_body_size is 1MB.
body_data = b"A" * 1500000
req = f"POST /uploads HTTP/1.1\r\nHost: localhost\r\nContent-Length: {len(body_data)}\r\nConnection: close\r\n\r\n".encode('utf-8')
res = send_raw_request(req + body_data)
if b'413' in res:
    print("✅ PASS: 413 Payload Too Large works.")
else:
    print(f"❌ FAIL: Expected 413 for body > 100 bytes, got: {res.split(b'\\r\\n')[0]}")
    failed += 1

# ---------------------------------------------------------
# Test 4: CRUD & Permissions
# ---------------------------------------------------------
print("\n--- 4. Methods, CRUD & Permissions ---")
# 405 Method Not Allowed (Delete on /autoindex where only GET is default)
res = send_request("DELETE /autoindex/ HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
if b'405' in res:
    print("✅ PASS: 405 Method Not Allowed works.")
else:
    print("❌ FAIL: Did not return 405 for DELETE on /autoindex/")
    failed += 1

# Create a dummy protected file for DELETE 403 test
try:
    os.makedirs("www/uploads", exist_ok=True)
    with open("www/uploads/protegido.txt", "w") as f:
        f.write("Secret")
    os.chmod("www/uploads/protegido.txt", 0o000)
    
    # Try deleting it (should return 403)
    res = send_request("DELETE /uploads/protegido.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    if b'403' in res:
        print("✅ PASS: 403 Forbidden when deleting file without permissions.")
    else:
        print(f"❌ FAIL: Expected 403 when deleting chmod 000 file, got: {res.split(b'\\r\\n')[0]}")
        failed += 1
finally:
    # Cleanup
    if os.path.exists("www/uploads/protegido.txt"):
        os.chmod("www/uploads/protegido.txt", 0o644)
        os.remove("www/uploads/protegido.txt")

# ---------------------------------------------------------
# Test 5: Binary Upload / Diff
# ---------------------------------------------------------
print("\n--- 5. Binary Corruption (Diff) ---")
# We'll upload some binary data (0x00 to 0xFF) and fetch it back.
binary_data = bytes(bytearray(range(256)) * 100) # 25KB of binary data
req_headers = f"POST /uploads/binary_test.bin HTTP/1.1\r\nHost: localhost\r\nContent-Length: {len(binary_data)}\r\nConnection: close\r\n\r\n".encode('utf-8')
res = send_raw_request(req_headers + binary_data)
if b'201' in res:
    print("✅ PASS: Binary file uploaded (201 Created).")
    
    # Now fetch it
    res = send_request("GET /uploads/binary_test.bin HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    if b'\r\n\r\n' in res:
        downloaded_body = res.split(b'\r\n\r\n', 1)[1]
        if downloaded_body == binary_data:
            print("✅ PASS: Binary diff is BIT-PERFECT! No corruption.")
        else:
            print(f"❌ FAIL: Binary corruption! Sent {len(binary_data)} bytes, received {len(downloaded_body)} bytes.")
            failed += 1
    else:
        print("❌ FAIL: Could not download the uploaded binary.")
        failed += 1
else:
    print(f"❌ FAIL: Could not upload binary, got {res.split(b'\\r\\n')[0]}")
    failed += 1

# Cleanup binary
if os.path.exists("www/uploads/binary_test.bin"):
    os.remove("www/uploads/binary_test.bin")

# ---------------------------------------------------------
# Test 6: Crash Resistance (Invented Methods)
# ---------------------------------------------------------
print("\n--- 6. Crash Resistance ---")
res = send_request("PUFF / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
if b'405' in res or b'400' in res or b'501' in res:
    print("✅ PASS: Invented method PUFF handled correctly (no crash).")
else:
    print(f"❌ FAIL: Unexpected response to PUFF method: {res.split(b'\\r\\n')[0]}")
    failed += 1

print("\n------------------------------------------------")
if failed == 0:
    print("🎉 ALL TESTS PASSED! Ready for 42 Evaluation.")
    sys.exit(0)
else:
    print(f"⚠️  {failed} TESTS FAILED. See log above.")
    sys.exit(1)
