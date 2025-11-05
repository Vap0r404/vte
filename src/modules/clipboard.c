#include "clipboard.h"
#include <stdlib.h>
#include <string.h>

static char *clip_content = NULL;
static ClipboardType clip_type = CLIP_CHAR;

void clipboard_init(void)
{
    clip_content = NULL;
    clip_type = CLIP_CHAR;
}

void clipboard_free(void)
{
    if (clip_content)
    {
        free(clip_content);
        clip_content = NULL;
    }
}

void clipboard_yank_char(const char *text)
{
    clipboard_free();
    if (text)
    {
        clip_content = strdup(text);
        clip_type = CLIP_CHAR;
    }
}

void clipboard_yank_line(const char *text)
{
    clipboard_free();
    if (text)
    {
        clip_content = strdup(text);
        clip_type = CLIP_LINE;
    }
}

char *clipboard_paste(void)
{
    if (!clip_content)
        return NULL;
    return strdup(clip_content);
}

ClipboardType clipboard_type(void)
{
    return clip_type;
}

int clipboard_has_content(void)
{
    return clip_content != NULL;
}
