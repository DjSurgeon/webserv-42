#!/usr/bin/env python3
import socket
import sys

def test_head_no_body():
    print("Test 1: HEAD request should have 0 bytes of body...")
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect(('127.0.0.1', 8080))
        s.sendall(b"HEAD / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
        response = b""
        while True:
            chunk = s.recv(4096)
            if not chunk:
                break
            response += chunk
        
        parts = response.split(b"\r\n\r\n", 1)
        headers = parts[0]
        body = parts[1] if len(parts) > 1 else b""
        
        if len(body) > 0:
            print(f"❌ FAIL: HEAD response contains a body of {len(body)} bytes:")
            print(body.decode('utf-8', errors='ignore'))
            return False
        else:
            print("✅ PASS: HEAD response contains no body.")
            return True
    except Exception as e:
        print(f"Error: {e}")
        return False
    finally:
        s.close()

def test_double_slash_uri():
    print("\nTest 2: GET //autoindex should be normalized and return a 200 OK (not 404)...")
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect(('127.0.0.1', 8080))
        s.sendall(b"GET //autoindex HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
        response = b""
        while True:
            chunk = s.recv(4096)
            if not chunk:
                break
            response += chunk
        
        headers = response.split(b"\r\n\r\n", 1)[0].decode('utf-8', errors='ignore')
        first_line = headers.split('\r\n')[0]
        
        if "404" in first_line:
            print(f"❌ FAIL: Server returned {first_line} instead of finding the directory index.")
            return False
        elif "200" in first_line:
            print(f"✅ PASS: Server returned {first_line}")
            return True
        else:
            print(f"❌ FAIL: Unexpected status: {first_line}")
            return False
    except Exception as e:
        print(f"Error: {e}")
        return False
    finally:
        s.close()

if __name__ == "__main__":
    print("Running tester bugs script...\n")
    pass1 = test_head_no_body()
    pass2 = test_double_slash_uri()
    
    if not pass1 or not pass2:
        sys.exit(1)
    
    print("\nAll tests passed successfully!")
    sys.exit(0)
