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
        -Add internal buffer for tracking text changes and saving it to the file instead of just printing characters to stdout.
        -Add proper file operations (save/edit).
