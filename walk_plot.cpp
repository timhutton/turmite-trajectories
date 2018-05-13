// Local:
#include "write_bmp.h"

// STL:
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>

/*

Do a (vN) random walk from a point on a line. Accumulate point location into matrix. If crosses the line then start again.
If exceeds N steps since last started then start again.

*/

void write_bmp_from_float(const std::string& filename, const float *g, const int w, const int h);

int main()
{
    const int X = 1000;
    const int Y = X;
    float grid[Y][X] = {0.0f};
    const int sx = X/2;
    const int sy = 0;
    int x = sx;
    int y = sy;
    const int N_DIRS = 4; // NESW
    const int dx[N_DIRS] = {0,1,0,-1};
    const int dy[N_DIRS] = {-1,0,1,0};
    
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis(0, N_DIRS-1);

    auto t1 = std::chrono::high_resolution_clock::now();
    
    const uint64_t million = 1000000ul;
    const uint64_t billion = 1000*million;
    const uint64_t trillion = 1000*billion;
    uint64_t iterations = 0;
    uint64_t n_steps = 0;
    for(; iterations<billion; ++iterations)
    {
        grid[y][x]++;
        if(n_steps>3000) {
            x = sx;
            y = sy;
            n_steps = 0;
        }
        else {
            int dir = dis(gen);
            x += dx[dir];
            y += dy[dir];
            if(x<0 || x>=X || y<0 || y>=Y)
            {
                x=sx;
                y=sy;
            }
            n_steps++;
        }
    }
    write_bmp_from_float("nplot.bmp",&grid[0][0],X,Y);

    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << iterations << " steps took: "
              << std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()
              << " seconds\n";
              

    return EXIT_SUCCESS;
}

void write_bmp_from_float(const std::string& filename, const float *g, const int w, const int h)
{
    write_bmp(filename,w,h,[&](int x,int y,char bgr[3]) {
        float val = g[y*w+x] / 10.0f;
        uint8_t c = val>255.0f ? 255 : ( val < 0.0f ? 0 : static_cast<uint8_t>(val) );
        bgr[0]=c; bgr[1]=c; bgr[2]=c;
    });
}
