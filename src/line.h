// C++ Standard Library:
#include <functional>

// do something along a Bresenham line
void line(int x0, int y0, int x1, int y1, std::function<void(int,int)> func)
{
    const int dx = abs(x1-x0);
    const int dy = abs(y1-y0);
    const int sx = x0<x1 ? 1 : -1;
    const int sy = y0<y1 ? 1 : -1; 
    int err = (dx>dy ? dx : -dy)/2;
    int e2;

    while(true)
    {
        func(x0,y0);
        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}
