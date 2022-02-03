#include <yed/plugin.h>

void scroll_buffer(int n_args, char **argv);

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_command(self, "scroll-buffer", scroll_buffer);

    return 0;
}

void scroll_buffer(int n_args, char **argv) {
    if (n_args == 1)
        YEXE("frame-scroll", argv[0]);
}
