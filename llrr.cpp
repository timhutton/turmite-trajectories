/*

LLRR turmite - What shape does it converge to?

10kx10k image easily contains 1trillion steps.
want plot of r against theta from origin, sampled whenever ant returns to origin and pattern is precisely symmetric
use to estimate growth rate (radius versus step) for the cardioid shape
want to see truchet tiles and hot tiles (from paper)
want to see 'average' trajectory in each loop, scale invariant maybe - is it a circle with one side fixed at the origin, of (increasing) random radius, modified with a Brownian walk constrained to leave the origin unchanged and also constrained to be symmetrical? that might give the cardioidness (a bulge outwards on each side of the fixed point) and explain why there's less of an indentation on the right (since no fixed point there but still forced to be symmetrical, or is the anticlockwise trajectories bulging from the fixed point). How to average trajectories? Draw onto a scaled image and accumulate? (buddhabrot style)
Are loops usually anti-clockwise? (as at beginning?) - No.
Where do loops usually cross?

*/

// Local:
#include "line.h"
#include "llrr.h"
#include "write_bmp.h"

// STL:
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>

const float PI=3.14159265358979f;

bool is_home(int x,int y,int sx,int sy,const uint8_t* g,int X,int Y)
{
    if( x!=sx || y!=sy) return false;
    for(int ty=sy;ty<Y;ty++)
    {
        for(int tx=0;tx<X;tx++)
        {
            int mirrored_ty = sy-1-(ty-sy);
            if( g[ty*X+tx] != g[mirrored_ty*X+tx] ) return false;
        }
    }
    return true;
}

void write_bmp_from_byte(const std::string& filename, const uint8_t *g, const int w, const int h);

int main()
{
    const int X = 900;
    const int Y = X;
    uint8_t grid[Y][X] = {0};
    uint8_t grid2[Y][X] = {0};
    const int sx = X/2;
    const int sy = Y/2;
    const int sdir = 2; // NESW=0123
    int x = sx;
    int y = sy;
    int dir = sdir;
    
    const uint64_t million = 1000000ul;
    const uint64_t billion = 1000*million;
    const uint64_t trillion = 1000*billion;

    auto t1 = std::chrono::high_resolution_clock::now();
    
    //grid[sy-1][sx-1]=1; // initial perturbation
    
    uint64_t iterations = 0;
    bool hit_edge = false;
    for(; iterations<billion; ++iterations)
    {
        LLRR(grid[y][x], dir, x, y);
        if(x<0 || x>=X || y<0 || y>=Y)
        {
            std::cout << "Hit edge.\n";
            hit_edge = true;
            break;
        }
    }
    write_bmp_from_byte("base.bmp",&grid[0][0],X,Y);
    
    // take measurements in polar coordinates?
    const bool take_polar_measurements = false;
    if(take_polar_measurements)
    {
        for(size_t theta = 0; theta < 360; theta++) {
            const float r = PI * theta / 180.0f;
            const int ex = int(sx + 100*std::max(X,Y)*cos(r));
            const int ey = int(sy + 100*std::max(X,Y)*sin(r));
            bool found = false;
            line( ex, ey, sx, sy, [&](int x,int y) {
                if( !found && x>=0 && x<X && y>=0 && y<Y && grid[y][x] > 0 ) {
                    printf("%f\n", hypot(sx-x,sy-y) );
                    found = true;
                }
            });
        }
    }
    
    // draw some loops?
    const bool draw_some_loops = false;
    if(draw_some_loops && !hit_edge)
    {
        int outLoop = 1;
        for(int iLoop=0;iLoop<1000000 && !hit_edge;iLoop++)
        {
            memcpy(&grid2[0][0],&grid[0][0],X*Y);
            uint64_t old_its = iterations;
            uint64_t crossing_its = 0;
            int crossing_x = 0;
            do
            {
                int old_y = y;
                LLRR(grid[y][x], dir, x, y);
                if(x<0 || x>=X || y<0 || y>=Y)
                {
                    std::cout << "Hit edge.\n";
                    hit_edge = true;
                    break;
                }
                if(grid2[y][x]<4) { grid2[y][x]=4; }
                else if(grid2[y][x]==4) { grid2[y][x]=5; }
                else { std::cout << "Path crossed itself?" << std::endl; }
                iterations++;
                if(old_y==sy && y==sy-1)
                {
                    crossing_its = iterations;
                    crossing_x = x;
                }
            }
            while(!hit_edge && !is_home(x,y,sx,sy,&grid[0][0],X,Y));
            if(iLoop>0 && iterations-old_its>10000)
            {
                std::cout << iterations << " crossing_x: " << crossing_x-sx << std::endl;
                if(iterations-crossing_its != crossing_its-old_its) {
                    std::cout << iterations-old_its << " != " << iterations-crossing_its << " + " << crossing_its-old_its << " ! " << std::endl;
                }
                std::ostringstream oss;
                oss << "loop_" << std::setfill('0') << std::setw(5) << outLoop++ << ".bmp";
                write_bmp_from_byte(oss.str(),&grid2[0][0],X,Y);
            }
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << iterations << " steps took: "
              << std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()
              << " seconds\n";
              

    return EXIT_SUCCESS;
}

void write_bmp_from_byte(const std::string& filename, const uint8_t *g, const int w, const int h)
{
    write_bmp(filename,w,h,[&](int x,int y, char bgr[3]) {
        switch(g[y*w+x]) {
            default:
            case 0: bgr[0]=0;         bgr[1]=0;         bgr[2]=0;   break;
            case 1: bgr[0]=(char)200; bgr[1]=0;         bgr[2]=0;   break;
            case 2: bgr[0]=0;         bgr[1]=(char)200; bgr[2]=0;   break;
            case 3: bgr[0]=0;         bgr[1]=0;         bgr[2]=(char)200; break;
            case 4: bgr[0]=(char)255; bgr[1]=(char)255; bgr[2]=(char)255; break;
        }
        return bgr;
    });
}
