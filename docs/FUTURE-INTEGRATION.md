# Future Integration: ternary-spreadsheet-c

## Current State
C implementation of a ternary spreadsheet engine — cells that are tiny ternary agents with values {-1, 0, +1}, supporting cell references, formula evaluation, and dependency tracking.

## Integration Opportunities

### With ternary-cell (Grid Compatibility)
Ternary spreadsheet cells ARE a simplified cell grid. The spreadsheet engine provides the formula and dependency tracking; `ternary-cell` provides the agent behavior. Together: cells that are both agents (autonomous behavior) and spreadsheet cells (computed values).

### With ternary-spreadsheet-python
Python for interactive development, C for deployment. Design spreadsheet layouts in Python, test them, then compile to C for ESP32 deployment. Cross-language spreadsheet portability: same formulas, same semantics, different execution environments.

### With ternary-database
Spreadsheet cells persist their values in ternary tables. Each cell is a row with columns for value, formula, and dependencies. `ternary-spreadsheet-c` computes; `ternary-database` stores. The spreadsheet IS the room state view.

## Potential in Mature Systems
In room-as-codespace, the ternary spreadsheet is the room dashboard. Each cell represents a room metric (energy, population, health). Formulas compute derived metrics. The C port runs the dashboard on edge devices — ESP32 displays room status on an LED matrix, driven by the ternary spreadsheet.

## Cross-Pollination Ideas
- Spreadsheet as a room programming model — non-programmers configure rooms by editing formulas
- Cell dependency graph as room data flow diagram
- Cross-language spreadsheet sharing: Python creates, C executes

## Dependencies for Next Steps
- Integration with ternary-cell for agent-enhanced spreadsheet cells
- Formula serialization format for Python → C transfer
- ESP32 display driver integration for room dashboards
