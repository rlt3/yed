#include "internal.h"

static void yed_add_new_buff(void) {
    yed_buffer **buff_ptr;

    if (ys->n_buffers == MAX_BUFFERS) {
        ERR;
    }

    buff_ptr   = ys->buff_list + ys->n_buffers++;
    *buff_ptr  = malloc(sizeof(**buff_ptr));
    **buff_ptr = yed_new_buff();
}

static void clear_output_buff(void) {
    ys->out_s.data[0] = 0;
    ys->out_s.used    = 1; /* 1 for NULL */
}

static void yed_init_output_stream(void) {
    ys->out_s.avail = 4096;
    ys->out_s.used  = 1; /* 1 for NULL */
    ys->out_s.data  = calloc(ys->out_s.avail, 1);
}

static int output_buff_len(void) { return ys->out_s.used; }

static void append_n_to_output_buff(const char *s, int n) {
    char *data_save;

    if (n) {
        if (ys->out_s.used + n > ys->out_s.avail) {
            ys->out_s.avail <<= 1;
            data_save         = ys->out_s.data;
            ys->out_s.data    = calloc(ys->out_s.avail, 1);

            memcpy(ys->out_s.data, data_save, ys->out_s.used);
            free(data_save);
        }

        strncat(ys->out_s.data, s, n);
        ys->out_s.used += n;
    }
}

static void append_to_output_buff(const char *s) {
    append_n_to_output_buff(s, strlen(s));
}

static void append_int_to_output_buff(int i) {
    char s[16];

    sprintf(s, "%d", i);

    append_to_output_buff(s);
}

static void flush_output_buff(void) {
    write(1, ys->out_s.data, ys->out_s.used);
    clear_output_buff();
}

static void yed_service_reload(void) {
    tree_reset_fns(yed_frame_id_t, yed_frame_ptr_t, ys->frames, strcmp);
    tree_reset_fns(yed_command_name_t, yed_command_t, ys->commands, strcmp);
    yed_reload_default_commands();
}

static int s_to_i(const char *s) {
    int i;

    sscanf(s, "%d", &i);

    return i;
}
