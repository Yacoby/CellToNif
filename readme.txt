====================================================================
CellToNif
====================================================================

Name:		CellToNif
Author(s):	Jacob Essex (aka Yacoby)
Version:	0.1
Last Updated:	July 08

====================================================================
LICENSE AND COPYRIGHT
====================================================================

See licence.txt


====================================================================
INTRODUCTION 
====================================================================

These programs allow you to convert a cell to a nif


====================================================================
INSTALLATION INSTRUCTIONS
====================================================================

Extract the files to your data files directory.

====================================================================
USING THE SOFTWARE
====================================================================

This is a comand line program, and you need to use the terminal to 
use the program.

However, if you right click on the file called ctn.bat (or just
c2n). It should be a window with a cog on it,

After rightclicking, click edit.

Delete the lines, and add in:
CellToNif "fileName" x y

Where filename is the file you want to load
cx is the x position of the cell
cy is the y position of the cell

So it would look something like this:

CellToNif "mod.esp" 3 -6

If you want to export multiple cells, you need to add in another couple of
arguments, and that is the 

CellToNif "mod.esp" x1 y1 x2 y2

So x1 and y1 are the lowest grid positions
And x2 and y2 are the highest grid positions

The program will then export a box, from x1,y1 to x2, y2

So another example of what we could put in c2d is
CellToNif "mod.esp"  5 -5 8 2

Which exports all the landscape from (5, -5) to (8, 2)



For those that know how to use a CLI:
CellToNif "data file" lowerx lowery [upperx] [uppery]


====================================================================
TEXTURES
====================================================================

If you are stuck as to what textures to use, you can use the output
from MGE

====================================================================
CONTRIBUTORS 
====================================================================  
PirateLord
