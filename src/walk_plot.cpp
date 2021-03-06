// Local:
#include "defs.h"
#include "line.h"
#include "write_bmp.h"

// C++ Standard Library:
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>

/*

Do a (vN) random walk from a point on a line. Accumulate point location into matrix. If crosses the line then start again.
If exceeds N steps since last started then start again.

TODO: extract radial profiles, pass into shape_test to confirm if can get constant growth from
      simple model (restrt on impact)

*/

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

    uint64_t iterations = 0;
    uint64_t n_steps = 0;
    for(; iterations<100*billion; ++iterations)
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
    write_bmp("walk_plot.bmp",X,Y,[&](int x,int y,unsigned char bgr[3]) {
        float val = grid[y][x] / 1000.0f;
        uint8_t c = val>255.0f ? 255 : ( val < 0.0f ? 0 : static_cast<uint8_t>(val) );
        bgr[0]=bgr[1]=bgr[2]=c;
    });

    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << iterations << " steps took: "
              << std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()
              << " seconds\n";

    // take measurements in polar coordinates?
    const bool take_polar_measurements = true;
    if(take_polar_measurements)
    {
        const float threshold = 128000.0f;
        for(size_t theta = 0; theta <= 180; theta++) {
            const float r = PI * theta / 180.0f;
            const int ex = int(sx + 100*std::max(X,Y)*cos(r));
            const int ey = int(sy + 100*std::max(X,Y)*sin(r));
            bool found = false;
            line( ex, ey, sx, sy, [&](int x,int y) {
                if( !found && x>=0 && x<X && y>=0 && y<Y && grid[y][x] > threshold ) {
                    printf("%f\n", hypot(sx-x,sy-y) );
                    found = true;
                }
            });
        }
    }

    return EXIT_SUCCESS;
}
