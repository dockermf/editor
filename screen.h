#ifndef MY_SCREEN_H
#define MY_SCREEN_H

void redraw_line(struct editor_buffer *buf, struct cursor_state *s);
void redraw_screen_full(struct editor_buffer *buf, struct cursor_state *s);

#endif
