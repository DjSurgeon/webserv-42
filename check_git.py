import subprocess

status = subprocess.run(['git', 'status'], capture_output=True, text=True)
with open('git_status.txt', 'w') as f:
    f.write(status.stdout)

branch = subprocess.run(['git', 'branch'], capture_output=True, text=True)
with open('git_branch.txt', 'w') as f:
    f.write(branch.stdout)

log = subprocess.run(['git', 'log', '-1'], capture_output=True, text=True)
with open('git_log.txt', 'w') as f:
    f.write(log.stdout)
