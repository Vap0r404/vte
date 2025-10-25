#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void config_init(EditorConfig *cfg)
{
    cfg->tab_width = 4;
    cfg->auto_indent = 1;
    cfg->show_line_numbers = 1;
    cfg->expand_tabs = 1;
    cfg->scroll_offset = 3;
    cfg->syntax_enabled = 1;
}

int config_load(EditorConfig *cfg, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return -1;

    char line[256];
    char status[256];
    while (fgets(line, sizeof(line), f))
    {
        /* Remove trailing newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        /* Skip empty lines and comments */
        if (line[0] == '\0' || line[0] == '#')
            continue;

        /* Parse as if it were a :set command */
        config_set(cfg, line, status, sizeof(status));
    }

    fclose(f);
    return 0;
}

int config_generate(const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f)
        return -1;

    fprintf(f, "# vte editor configuration file\n");
    fprintf(f, "# Place in current directory or home directory\n\n");
    fprintf(f, "# Tab width (1-16)\n");
    fprintf(f, "tab_width=4\n\n");
    fprintf(f, "# Auto-indent new lines\n");
    fprintf(f, "auto_indent=on\n\n");
    fprintf(f, "# Show line numbers\n");
    fprintf(f, "line_numbers=on\n\n");
    fprintf(f, "# Convert tabs to spaces\n");
    fprintf(f, "expand_tabs=on\n\n");
    fprintf(f, "# Minimum lines to keep above/below cursor (0-20)\n");
    fprintf(f, "scroll_offset=3\n\n");
    fprintf(f, "# Enable syntax highlighting\n");
    fprintf(f, "syntax=on\n");

    fclose(f);
    return 0;
}

static int parse_bool(const char *value)
{
    if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0 ||
        strcmp(value, "on") == 0 || strcmp(value, "yes") == 0)
        return 1;
    return 0;
}

int config_set(EditorConfig *cfg, const char *cmd, char *status_out, size_t status_len)
{
    /* Expected format: "setting=value" or just "setting" for booleans */
    char buf[256];
    strncpy(buf, cmd, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    /* Trim leading whitespace */
    char *p = buf;
    while (*p && isspace((unsigned char)*p))
        p++;

    /* Find the '=' separator */
    char *eq = strchr(p, '=');
    char *setting = p;
    char *value = NULL;

    if (eq)
    {
        *eq = '\0';
        value = eq + 1;
        /* Trim trailing whitespace from setting name */
        char *end = eq - 1;
        while (end > setting && isspace((unsigned char)*end))
            *end-- = '\0';
    }

    /* Boolean settings can be specified without value */
    if (!value || value[0] == '\0')
        value = "1";

    /* Match setting and update config */
    if (strcmp(setting, "tabwidth") == 0 || strcmp(setting, "tab_width") == 0)
    {
        int val = atoi(value);
        if (val > 0 && val <= 16)
        {
            cfg->tab_width = val;
            snprintf(status_out, status_len, "tab_width = %d", val);
            return 0;
        }
        snprintf(status_out, status_len, "Invalid tab_width (must be 1-16)");
        return -1;
    }
    else if (strcmp(setting, "autoindent") == 0 || strcmp(setting, "auto_indent") == 0)
    {
        cfg->auto_indent = parse_bool(value);
        snprintf(status_out, status_len, "auto_indent = %s", cfg->auto_indent ? "on" : "off");
        return 0;
    }
    else if (strcmp(setting, "linenumbers") == 0 || strcmp(setting, "line_numbers") == 0)
    {
        cfg->show_line_numbers = parse_bool(value);
        snprintf(status_out, status_len, "line_numbers = %s", cfg->show_line_numbers ? "on" : "off");
        return 0;
    }
    else if (strcmp(setting, "expandtabs") == 0 || strcmp(setting, "expand_tabs") == 0)
    {
        cfg->expand_tabs = parse_bool(value);
        snprintf(status_out, status_len, "expand_tabs = %s", cfg->expand_tabs ? "on" : "off");
        return 0;
    }
    else if (strcmp(setting, "scrolloffset") == 0 || strcmp(setting, "scroll_offset") == 0)
    {
        int val = atoi(value);
        if (val >= 0 && val <= 20)
        {
            cfg->scroll_offset = val;
            snprintf(status_out, status_len, "scroll_offset = %d", val);
            return 0;
        }
        snprintf(status_out, status_len, "Invalid scroll_offset (must be 0-20)");
        return -1;
    }
    else if (strcmp(setting, "syntax") == 0 || strcmp(setting, "syntax_enabled") == 0)
    {
        cfg->syntax_enabled = parse_bool(value);
        snprintf(status_out, status_len, "syntax = %s", cfg->syntax_enabled ? "on" : "off");
        return 0;
    }

    snprintf(status_out, status_len, "Unknown setting: %s", setting);
    return -1;
}

void config_show(const EditorConfig *cfg, char *out, size_t len)
{
    snprintf(out, len,
             "tab_width=%d auto_indent=%s line_numbers=%s expand_tabs=%s scroll_offset=%d syntax=%s",
             cfg->tab_width,
             cfg->auto_indent ? "on" : "off",
             cfg->show_line_numbers ? "on" : "off",
             cfg->expand_tabs ? "on" : "off",
             cfg->scroll_offset,
             cfg->syntax_enabled ? "on" : "off");
}
