#include "iter8.h"
#include <stddef.h>
#include <stdlib.h>

struct list8 *__list8_next(struct list8 *node) {
    if (node) {
        list8 *next = node->next;
        free(node);
        return next;
    }
    return NULL;
}

struct list8 *__list8_new(struct list8 init) {
    list8 *hd = calloc(1, sizeof(list8));
    *hd = init;
    return hd;
}
