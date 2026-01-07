/**
 * Byte data list handle
 */
typedef struct list8 list8;

typedef struct stack8 stack8;

int list8_next(list8 *, unsigned char *);
int stack8_pop(stack8 *, unsigned char *);

#define next8(obj) _Generic((obj), \
    struct list8: list8_next, \
    struct stack8: stack8_next \
)
