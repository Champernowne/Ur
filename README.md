# Ur
A CLI Implementation of the Royal Game of Ur
The project builds a Windows CLI executable Ur.exe. 

The execuatble is invoked from the command line with two arguments, which are file names. 
When the executable is first invoked, the files named should not exist. They will be written this takes several hours.
On subsequent invocations the existing files will be read.

Once initialization is complete, a diagram of the board is displayed.
Posiions on the board are named by a column and a row and a column. Rows are numbered from 0 at the top to 2 at the bottom, Columns from 0 on the right to 7 on the left.
White's side of the board is row 0.

There is little command documentation.

throw     Throws the dice

moveto    Specifies a move, once the dice have been thrown

move      Asks the computer to move

pass      Acknowledges there is no legal move and ends a turn

