---
name: Task
about: Standard template for creating small, specific, testable tasks.
title: '[MODULE] Brief description in infinitive form'
labels: enhancement
assignees: ''
---

## 🎯 1. Objective (User Story)
*As a developer on the team, I want [task description] so that the server can [benefit or functionality].*

---

## 📝 2. Technical Task List (Checklist)
- [ ] Create/modify necessary files (.hpp / .cpp).
- [ ] Implement logic strictly adhering to C++98.
- [ ] Handle errors using exceptions or control codes without crashing the server.

---

## 🧪 3. Acceptance Criteria (Definition of Done)
- [ ] Code compiles without any warnings using `c++ -Wall -Wextra -Werror -std=c++98`.
- [ ] Successfully tested in isolation (local test) and works as expected.
- [ ] No memory leaks or open file descriptors left behind.
- [ ] Pull Request has been reviewed and approved by at least 1 team member.
