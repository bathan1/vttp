#include "iter8.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>


struct list *__list_next(struct list *ls) {
    if (ls) {
        return ls->next;
    }
    return NULL;
}

const char *__list_peek(struct list *ls) {
    if (ls) {
        return ls->buffer;
    }
    return NULL;
}

void __list_done(struct list *ls) {
    if (!ls)
        return;

    list *next = ls->next;
    list *prev = ls->prev;

    free(ls);

    while (next) {
        list *next_next = next->next;
        free(next);
        next = next_next;
    }

    while (prev) {
        list *prev_prev = prev->prev;
        free(prev);
        prev = prev_prev;
    }
}

int main() {
    list hd = {
        .buffer = "hello",
        .length = sizeof("hello") - 1,
        .prev = NULL
    };

    list md = {
        .buffer = " ",
        .length = sizeof(" ") - 1,
        .prev = &hd
    };

    list tl = {
        .buffer = "world!\n",
        .length = sizeof("world!\n") - 1,
        .prev = &md,
        .next = NULL
    };

    hd.next = &md;
    md.next = &tl;

    list *_hd = malloc(sizeof(list));
    *_hd = hd;
    list *_md = malloc(sizeof(list));
    *_md = md;
    list *_tl = malloc(sizeof(list));
    *_tl = tl;

    _hd->prev = NULL;
    _hd->next = _md;
    _md->prev = _hd;
    _md->next = _tl;
    _tl->prev = _md;
    _tl->next = NULL;

    list *ls = &hd;

    while (ls != NULL) {
        printf("%s", peek(ls));
        ls = next(ls);
    }

    ls = _hd;
    while (ls != NULL) {
        printf("%s", peek(ls));
        ls = next(ls);
    }
    done(_tl);

    return 0;
}
