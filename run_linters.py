import os
import subprocess

files = []
for root, _, filenames in os.walk('src'):
    for filename in filenames:
        if filename.endswith('.cpp') or filename.endswith('.hpp'):
            files.append(os.path.join(root, filename))

for root, _, filenames in os.walk('include'):
    for filename in filenames:
        if filename.endswith('.cpp') or filename.endswith('.hpp'):
            files.append(os.path.join(root, filename))

# Run clang-format
for f in files:
    subprocess.run(['clang-format', '-i', f])

# Run cpplint
with open('lint_output.txt', 'w') as out:
    for f in files:
        result = subprocess.run(['cpplint', f], capture_output=True, text=True)
        out.write(result.stderr)
