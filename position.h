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
};

#endif // POSITION_H
