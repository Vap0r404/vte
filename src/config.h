#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>

typedef struct
{
    int tab_width;         /* Number of spaces for a tab */
    int auto_indent;       /* Auto-indent new lines */
    int show_line_numbers; /* Display line numbers (always on for now) */
    int expand_tabs;       /* Convert tabs to spaces */
    int scroll_offset;     /* Min lines to keep above/below cursor when scrolling */
    int syntax_enabled;    /* Enable syntax highlighting */
} EditorConfig;

/* Initialize config with defaults */
void config_init(EditorConfig *cfg);

/* Load config from file (e.g., .vterc in current directory or home) */
int config_load(EditorConfig *cfg, const char *path);

/* Generate a default config file */
int config_generate(const char *path);

/* Parse a :set command and update config */
int config_set(EditorConfig *cfg, const char *cmd, char *status_out, size_t status_len);

/* Get a human-readable list of current settings */
void config_show(const EditorConfig *cfg, char *out, size_t len);

#endif /* CONFIG_H */
