import subprocess

files_to_format = [
    'include/handlers/CgiHandler.hpp',
    'include/network/EventLoop.hpp',
    'src/handlers/CgiHandler.cpp',
    'src/network/EventLoop.cpp',
    'src/network/EventLoop_read.cpp'
]

for f in files_to_format:
    subprocess.run(['clang-format', '-i', f])
