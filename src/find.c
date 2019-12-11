#include "internal.h"

void yed_init_search(void) {
    ys->replace_markers       = array_make(array_t);
    ys->replace_save_lines    = array_make(yed_line*);
    ys->replace_working_lines = array_make(yed_line*);
}

int search_can_move_cursor(void) {
    return yed_get_var("enable-search-cursor-move") != NULL;
}

void yed_search_line_handler(yed_event *event) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    yed_attrs  *attr, search, search_cursor;
    char       *line_data,
               *line_data_start,
               *line_data_end;
    int         i,
                search_len,
                data_len;

    if (!ys->current_search) {
        return;
    }

    if (!event->frame) {
        return;
    }

    frame = event->frame;

    if (frame != ys->active_frame) {
        return;
    }

    if (!frame->buffer) {
        return;
    }

    buff       = frame->buffer;
    line       = yed_buff_get_line(buff, event->row);
    data_len   = array_len(line->chars);
    search_len = strlen(ys->current_search);

    if (!data_len || !search_len)    { return; }

    line_data     = line_data_start = array_data(line->chars);
    line_data_end = line_data + data_len;

    search        = yed_active_style_get_search();
    search_cursor = yed_active_style_get_search_cursor();

    for (; line_data != line_data_end; line_data += 1) {
        if (strncmp(ys->current_search, line_data, search_len) == 0) {
            if (event->row == frame->cursor_line
            && (line_data - line_data_start) + 1 == frame->cursor_col) {
                for (i = 0; i < search_len; i += 1) {
                    attr = array_item(event->line_attrs, (line_data - line_data_start) + i);
                    if (ys->active_style) {
                        yed_combine_attrs(attr, &search_cursor);
                    } else {
                        attr->flags ^= ATTR_INVERSE;
                    }
                }
            } else {
                for (i = 0; i < search_len; i += 1) {
                    attr  = array_item(event->line_attrs, (line_data - line_data_start) + i);
                    if (ys->active_style) {
                        yed_combine_attrs(attr, &search);
                    } else {
                        attr->flags ^= ATTR_INVERSE;
                    }
                }
            }
        }
    }
}

int yed_find_next(int row, int col, int *row_out, int *col_out) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    char       *line_data,
               *line_data_start;
    int         i,
                r,
                search_len,
                data_len,
                junk_row, junk_col;

    if (!ys->current_search)    { return 0; }
    if (!ys->active_frame)      { return 0; }

    frame = ys->active_frame;

    if (!frame->buffer)    { return 0; }

    buff       = frame->buffer;

    if (buff->has_selection && !search_can_move_cursor()) {
        *row_out = row;
        *col_out = col;
        row_out  = &junk_row;
        col_out  = &junk_col;
    }

    search_len = strlen(ys->current_search);

    if (!search_len)    { return 0; }

    r = row;
    bucket_array_traverse_from(buff->lines, line, row - 1) {
        data_len = array_len(line->chars);

        if (!data_len) {
            r += 1;
            continue;
        }

        line_data = line_data_start = array_data(line->chars);

        if (r == row)    { i = col - 1; }
        else             { i = 0;       }

        for (; i < data_len - search_len + 1; i += 1) {
            if (strncmp(ys->current_search, line_data + i, search_len) == 0) {
                if (r != row || i + 1 != col) {
                    *row_out = r;
                    *col_out = i + 1;
                    return 1;
                }
            }
        }

        r += 1;
    }

    r = 1;
    bucket_array_traverse(buff->lines, line) {
        data_len = array_len(line->chars);

        if (!data_len) {
            r += 1;
            continue;
        }

        line_data = line_data_start = array_data(line->chars);

        if (r == row)    { data_len = col - 1; }

        for (i = 0; i < data_len - search_len + 1; i += 1) {
            if (strncmp(ys->current_search, line_data + i, search_len) == 0) {
                if (r != row || i + 1 != col) {
                    *row_out = r;
                    *col_out = i + 1;
                    return 1;
                }
            }
        }

        r += 1;
    }

    return 0;
}

int yed_find_prev(int row, int col, int *row_out, int *col_out) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_line   *line;
    char       *line_data,
               *line_data_start;
    int         i,
                r,
                search_len,
                data_len,
                junk_row, junk_col;

    if (!ys->current_search)    { return 0; }
    if (!ys->active_frame)      { return 0; }

    frame = ys->active_frame;

    if (!frame->buffer)    { return 0; }

    buff       = frame->buffer;

    if (buff->has_selection && !search_can_move_cursor()) {
        *row_out = row;
        *col_out = col;
        row_out  = &junk_row;
        col_out  = &junk_col;
    }

    search_len = strlen(ys->current_search);

    if (!search_len)    { return 0; }

    for (r = row; r > 0; r -= 1) {
        line = yed_buff_get_line(buff, r);

        data_len = array_len(line->chars);

        if (!data_len) {
            continue;
        }

        line_data = line_data_start = array_data(line->chars);

        if (r == row)    { i = col - 2 - search_len;  }
        else             { i = data_len - search_len; }

        for (; i >= 0; i -= 1) {
            if (strncmp(ys->current_search, line_data + i, search_len) == 0) {
                if (r != row || i + 1 != col) {
                    *row_out = r;
                    *col_out = i + 1;
                    return 1;
                }
            }
        }
    }

    for (r = bucket_array_len(buff->lines); r > row; r -= 1) {
        line = yed_buff_get_line(buff, r);

        data_len = array_len(line->chars);

        if (!data_len) {
            continue;
        }

        line_data = line_data_start = array_data(line->chars);

        for (i = data_len - search_len; i >= 0; i -= 1) {
            if (strncmp(ys->current_search, line_data + i, search_len) == 0) {
                if (r != row || i + 1 != col) {
                    *row_out = r;
                    *col_out = i + 1;
                    return 1;
                }
            }
        }
    }

    return 0;
}
