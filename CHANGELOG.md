## 2025-07-30
### -Fixed array memory corruption.
### -Todo: proper cursor control and movement; handle crashes/unexpected exits.

## 2025-07-30
### -State struct is used without typedef now.
### -Partial code styling change.

## 2025-08-04
### -Pushed changes from July 30th.

## 2025-08-05
### -Made progress towards better cursor movement control.

## 2025-08-08
### -Made a working cursor movement, will need to polish a bit.
### -Cursor movement rules refactored.
### -Slight changes to code style in [utils.c](utils.c).

## 2025-08-09
### -Changed backspace logic to go on the previous line if the cursor is at the beginning of the current line.

## 2025-08-10 (not committed)
### -Added internal buffer, now i need to integrate it without breaking other parts of the code.
### -Slightly changed code visually (empty lines, more distinctive code flow?).

## 2025-08-12
### -Changed text buffer control a bit, overall made some progress.

## 2025-08-13
### -Fixed enter key press not reallocating buffer memory size to write on new lines.
### -Fixed not tracking/changing lines' lengths in the buffer.

## 2025-08-14
### -Fixed buffer and cursor structs, now ALL line related stuff is accessed via buffer struct instead of the cursor struct.
### -Removed most (if not all) unnecessary functions.
### -Made it so it adds null terminating character at the end of a line in the buffer.

## 2025-08-16
### -Changed initial line length's memory size (2 -> 256 bytes) to prevent often realloc calls and hence reducing allocation failure cases.
### -Added memmove for character insertion + deletion.
### -Removed todo list from here.

## 2025-08-17
### -Fixed unnoticed memset bug (it didn't set the pointer's extension with data like size\_t due to size\_t being 4 or 8 bytes, while memset sets individual bytes, so it was changed to for loop assignment).
### -Added line refresh on the screen, takes characters from the text buffer and spits them to stdout (terminal).
### -Added todo list in [main.c](main.c).

## 2025-08-18
### -Created [helpers.h](helpers.h) and [helpers.c](helpers.c) files to clean up [utils.c](utils.c).
### -Made it possible to pass a filename as an argument to the editor and it checks if the filename already exists, will help with file operations in the future.
