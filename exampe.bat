@ECHO OFF
REM: Usage:
REM:		"data file" lowerx lowery [upperx] [uppery]

REM: Example of getting a single cell
CellToNif "Morrowind.esm" 0 1

REM: Another example of getting multiple cells
CellToNif "Bloodmoon.esm" -19 19 -18 20