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
### -TODO:
\        ~~-Add internal buffer for tracking text changes and saving it to the file instead of just printing characters to stdout.~~
\        -Fix any issues after adding the new buffer and make the buffer work.
\            ~~-Buffer change on enter key press should be fixed (needs to actually increase allocated memory).~~
\        -Remove unnecessary functions in the code.
\        -Add proper file operations (save/edit).

## 2025-08-10 (not committed)
### -Added internal buffer, now i need to integrate it without breaking other parts of the code.
### -Slightly changed code visually (empty lines, more distinctive code flow?).

## 2025-08-12
### -Changed text buffer control a bit, overall made some progress.

## 2025-08-13
### -Fixed enter key press not reallocating buffer memory size to write on new lines.
### -Fixed not tracking/changing lines' lengths in the buffer.
