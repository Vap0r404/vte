#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFERS 16
static Buffer buffers[MAX_BUFFERS];
static size_t buf_count = 0;
static int cur_buf = 0;

static void buffer_init(Buffer *b)
{
    b->count = 0;
    b->path = NULL;
    b->dirty = 0;
}

void buffer_pool_init(void)
{
    for (int i = 0; i < MAX_BUFFERS; ++i)
        buffer_init(&buffers[i]);
    buf_count = 1;
    cur_buf = 0;
}

Buffer *buffer_current(void)
{
    return &buffers[cur_buf];
}

size_t buffer_count(void)
{
    return buf_count;
}

int buffer_index(void)
{
    return cur_buf;
}

int buffer_open_file(const char *path)
{
    if (!path)
        return -1;
    /* If file already open, switch to it */
    for (size_t i = 0; i < buf_count; ++i)
    {
        if (buffers[i].path && strcmp(buffers[i].path, path) == 0)
        {
            cur_buf = (int)i;
            return (int)i;
        }
    }
    if (buf_count >= MAX_BUFFERS)
        return -1;
    Buffer *b = &buffers[buf_count];
    buffer_init(b);
    FILE *f = fopen(path, "rb");
    if (!f)
        return -1;
    char linebuf[8192];
    while (fgets(linebuf, sizeof(linebuf), f))
    {
        size_t len = strlen(linebuf);
        while (len > 0 && (linebuf[len - 1] == '\n' || linebuf[len - 1] == '\r'))
            linebuf[--len] = '\0';
        b->lines[b->count] = malloc(len + 1);
        strcpy(b->lines[b->count], linebuf);
        b->count++;
        if (b->count >= MAX_LINES)
            break;
    }
    if (b->count == 0)
    {
        b->lines[0] = malloc(1);
        b->lines[0][0] = '\0';
        b->count = 1;
    }
    fclose(f);
    b->path = strdup(path);
    b->dirty = 0;
    cur_buf = (int)buf_count;
    buf_count++;
    return cur_buf;
}

int buffer_save_current(const char *path)
{
    Buffer *b = buffer_current();
    const char *p = path ? path : b->path;
    if (!p)
        return -1;
    FILE *f = fopen(p, "wb");
    if (!f)
        return -1;
    for (size_t i = 0; i < b->count; ++i)
        fprintf(f, "%s\n", b->lines[i]);
    fclose(f);
    if (b->path)
        free(b->path);
    b->path = strdup(p);
    b->dirty = 0;
    return 0;
}

int buffer_next(void)
{
    if (buf_count == 0)
        return 0;
    cur_buf = (cur_buf + 1) % (int)buf_count;
    return cur_buf;
}

int buffer_prev(void)
{
    if (buf_count == 0)
        return 0;
    cur_buf = (cur_buf - 1 + (int)buf_count) % (int)buf_count;
    return cur_buf;
}

void buffer_free_all(void)
{
    for (size_t i = 0; i < buf_count; ++i)
    {
        Buffer *b = &buffers[i];
        for (size_t j = 0; j < b->count; ++j)
            free(b->lines[j]);
        if (b->path)
            free(b->path);
    }
}
