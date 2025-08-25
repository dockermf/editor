#ifndef MY_CURSOR_H
#define MY_CURSOR_H

bool can_move_cursor(struct editor_buffer* buf, struct cursor_state* s, const int dx, const int dy);
bool next_line_has_less_cols(struct editor_buffer* buf, struct cursor_state* s, const int dy);
void adjust_pos_to_lastchar(struct editor_buffer* buf, struct cursor_state* s, const int dy);
void cursor_update_coords(struct cursor_state* s, const int dx, const int dy);
void display_cursor_position(struct cursor_state* s);

#endif
