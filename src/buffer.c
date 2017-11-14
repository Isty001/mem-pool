#include "internals.h"


Buffer *buffer_new(size_t size)
{
    Buffer *buff = malloc(sizeof(Buffer));
    buff->start = malloc(size);
    buff->current = buff->start;
    buff->next = NULL;
    buff->end = buff->current + size;

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
