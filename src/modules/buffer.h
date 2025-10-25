#ifndef VTE_BUFFER_H
#define VTE_BUFFER_H

#include <stddef.h>

#define MAX_LINES 65536

typedef struct Buffer
{
    char *lines[MAX_LINES];
    size_t count;
    char *path; /* optional filename for this buffer */
    int dirty;  /* modified since last save */
} Buffer;

/* Buffer pool management */
void buffer_pool_init(void);
Buffer *buffer_current(void);
size_t buffer_count(void);
int buffer_open_file(const char *path); /* returns index or -1 on error */
int buffer_save_current(const char *path);
int buffer_next(void);  /* switch to next buffer, returns new index */
int buffer_prev(void);  /* switch to previous buffer */
int buffer_index(void); /* current buffer index */
void buffer_free_all(void);

#endif /* VTE_BUFFER_H */
