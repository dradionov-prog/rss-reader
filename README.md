# RSS Reader (Вариант А)

Консольная читалка RSS/Atom с TUI на FTXUI.
См. `DESIGN.md` для архитектуры.

## Сборка

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build