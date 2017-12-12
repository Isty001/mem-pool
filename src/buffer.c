#include "internals.h"


Buffer *buffer_new(size_t size)
{
    Buffer *buff = malloc(sizeof(Buffer));
    if (!buff) {
        return NULL;
    }

    buff->start = malloc(size);
    if (!buff->start) {
        free(buff);
        return NULL;
    }

    buff->curr_ptr = buff->start;
    buff->next = NULL;
    buff->prev_ptr = NULL;
    buff->end = buff->curr_ptr + size;

    return buff;
}

Buffer *buffer_list_find(Buffer *head, void *ptr)
{
    while (head) {
        if (buffer_has(head, ptr)) {
            return head;
        }
        head = head->next;
    }

    return NULL;
}

void buffer_list_destroy(Buffer *head)
{
    Buffer *buff;

    while (head) {
        buff = head;
        head = head->next;
        free(buff->start);
        free(buff);
    }
}
