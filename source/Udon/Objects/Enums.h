#ifndef UDON_OBJECTS_ENUMS_H
#define UDON_OBJECTS_ENUMS_H

namespace Display { enum {
    NONE = 0,
    BLOCK = 1,
    INLINE = 2,
    INLINE_BLOCK = 3,
};};

namespace ValueType { enum {
    AUTO,
    PIXELS,
    POINTS,
    PERCENT,
    FILL_REMAINING,
};};

typedef struct {
    int Type;
    float Value;
} Value;

#endif /* UDON_OBJECTS_ENUMS_H */
