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

// STL:
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>

const float PI=3.14159265358979f;

void LLRR(uint8_t& state, int& dir, int& x, int& y)
{
    const uint8_t N_STATES = 4;
    const int N_DIRS = 4;
    const int d_dir[N_STATES] = {-1,-1,1,1}; // LLRR
    const int dx[N_DIRS]={0,1,0,-1};
    const int dy[N_DIRS]={-1,0,1,0};
    dir = (dir+d_dir[state]+N_DIRS) % N_DIRS;
    state = (state+1) % N_STATES;
    x += dx[dir];
    y += dy[dir];
}

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

void write_bmp_from_byte(const std::string& filename, const uint8_t *g, const int w, const int h);

int main()
{
    const int X = 8000;
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
    
    uint64_t iterations = 0;
    bool hit_edge = false;
    for(; iterations<trillion; ++iterations)
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
    const bool take_polar_measurements = true;
    if(take_polar_measurements)
    {
        // every degree in [0,180]
        for(float theta = 0.0f; theta <= 180.0f; theta += 1.0f) {
            const float r = PI * theta / 180.0f;
            const int ex = int(sx + 100*std::max(X,Y)*cos(r));
            const int ey = int(sy + 100*std::max(X,Y)*sin(r));
            bool found = false;
            line( ex, ey, sx, sy, [&](int x,int y) {
                if( !found && x>=0 && x<X && y>=0 && y<Y && grid[y][x] > 0 ) {
                    printf("%f %f\n", theta, hypot(sx-x,sy-y) );
                    found = true;
                }
            });
        }
        write_bmp_from_byte("base2.bmp",&grid[0][0],X,Y);
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
    // g[w*h] is row-major indexed color values, where w is the image width, h is image height
    int filesize = 54 + 3*w*h;
    char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    char bmppad[3] = {0,0,0};

    bmpfileheader[ 2] = (unsigned char)(filesize    );
    bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);

    bmpinfoheader[ 4] = (unsigned char)(       w    );
    bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
    bmpinfoheader[ 6] = (unsigned char)(       w>>16);
    bmpinfoheader[ 7] = (unsigned char)(       w>>24);
    bmpinfoheader[ 8] = (unsigned char)(       h    );
    bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
    bmpinfoheader[10] = (unsigned char)(       h>>16);
    bmpinfoheader[11] = (unsigned char)(       h>>24);

    std::fstream output(filename, std::ios::out | std::ios::binary);
    output.write(bmpfileheader,14);
    output.write(bmpinfoheader,40);
    char bgr[3];
    for(int i=0; i<h; i++)
    {
        for(int j=0;j<w;j++)
        {
            // TODO: allow user of function to change colormap, currently: [0,1,2,3]->[black,blue,green,red]
            switch(g[(h-i-1)*w+j]) {
                default:
                case 0: bgr[0]=0;         bgr[1]=0;         bgr[2]=0;   break;
                case 1: bgr[0]=(char)200; bgr[1]=0;         bgr[2]=0;   break;
                case 2: bgr[0]=0;         bgr[1]=(char)200; bgr[2]=0;   break;
                case 3: bgr[0]=0;         bgr[1]=0;         bgr[2]=(char)200; break;
                case 4: bgr[0]=(char)255; bgr[1]=(char)255; bgr[2]=(char)255; break;
            }
            output.write(bgr,3);
        }
        output.write(bmppad,(4-(w*3)%4)%4);
    }
}
