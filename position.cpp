#include "position.h"

Position Position::toPx(float left, float down, float top, float right, int vw, int wh) {
    int l = static_cast<int>((left  + 1.0f) * 0.5f * vw);
    int r = static_cast<int>((right + 1.0f) * 0.5f * vw);

    int d = static_cast<int>((down + 1.0f) * 0.5f * wh);
    int t = static_cast<int>((top  + 1.0f) * 0.5f * wh);

    d = wh - d;
    t = wh - t;

    return Position(l, d, t, r);
}

Position Position::rotateCW90(const Position& pos) {
    return Position(pos.down, -pos.right, -pos.left, pos.top);
}
