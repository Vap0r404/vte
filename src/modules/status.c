#include "status.h"
#include <stdio.h>
#include <string.h>

void status_format(char *out, size_t len, const char *mode, const char *filename,
                   int buf_index, size_t buf_total, int is_dirty,
                   size_t line, size_t col, int term_cols)
{
    char fbuf[256];
    const char *fname = (filename && filename[0]) ? filename : "[No file]";

    if (is_dirty)
        snprintf(fbuf, sizeof(fbuf), "%s [%d/%zu] [+]", fname, buf_index + 1, buf_total);
    else
        snprintf(fbuf, sizeof(fbuf), "%s [%d/%zu]", fname, buf_index + 1, buf_total);

    /* Build position info */
    char posinfo[32];
    snprintf(posinfo, sizeof(posinfo), "%zu:%zu", line + 1, col + 1);

    /* Calculate space available for status message */
    int mode_len = strlen(mode);
    int fbuf_len = strlen(fbuf);
    int pos_len = strlen(posinfo);
    int fixed_len = mode_len + fbuf_len + pos_len + 10; /* "-- -- " and spacing */

    /* Format output with position right-aligned */
    int padding = term_cols - fixed_len;
    if (padding < 0)
        padding = 0;

    snprintf(out, len, "-- %s -- %s%*s%s", mode, fbuf, padding, "", posinfo);
}
