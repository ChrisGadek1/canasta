#ifndef POSITION_H
#define POSITION_H

class Position {
public:
    float left, down, top, right;
    Position(float left, float down, float top, float right): left(left), down(down), top(top), right(right) {}
    void setLeft(float left) { this->left = left; }
    void setRight(float right) { this->right = right; }
    void setDown(float down) { this->down = down; }
    void setTop(float top) { this->top = top; }
    static Position toPx(float left, float down, float top, float right, int vw, int wh);
    static Position rotateCW90(const Position& pos);
    static Position lerp(const Position& from, const Position& to, float progress);
    bool operator==(const Position& b) const { return left == b.left && down == b.down && top == b.top && right == b.right; }
};

#endif // POSITION_H
