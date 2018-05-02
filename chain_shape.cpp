// STL:
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

/*

Do a (vN) random walk from a point on the edge of an area. If crosses the line then start again.
If hits the curved wall then accumulate there and start again.

Jitter each block in the chain wall, biased to grow outwards where the value is higher than its neighbors,
and inwards if less. Ends of chain are constrained to lie on first row.

Should get half-nephroid shape. 

After that, add secondary hits (restricted to right of line from hit to start). Get LLRR shape?

*/

void write_bmp_from_float(const std::string& filename, const uint64_t *g, const int w, const int h, const float gain);

int main()
{
    const int X = 1000;
    const int Y = 400;
    uint64_t grid[Y][X] = {0};
    bool is_chain[Y][X] = {false};
    const int sx = X/2;
    const int sy = 0;
    int x = sx;
    int y = sy;
    const int N_DIRS = 4; // NESW
    const int dx[N_DIRS] = {0,1,0,-1};
    const int dy[N_DIRS] = {-1,0,1,0};
    const int N_CHAIN_POINTS = 400;
    int chain[N_CHAIN_POINTS][2];
    
    // initialize chain to be a half square
    int cx = sx - N_CHAIN_POINTS / 4;
    int cy = sy;
    for(int i=0;i<N_CHAIN_POINTS;i++)
    {
        chain[i][0]=cx;
        chain[i][1]=cy;
        is_chain[cy][cx]=true;
        if(i>=3*N_CHAIN_POINTS/4-2) { cy -= 1; }
        else if(i>N_CHAIN_POINTS/4) { cx += 1; }
        else cy += 1;
    }
    
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis(0, N_DIRS-1);

    auto t1 = std::chrono::high_resolution_clock::now();
    
    const uint64_t million = 1000000ul;
    const uint64_t billion = 1000*million;
    const uint64_t trillion = 1000*billion;
    uint64_t iterations = 0;
    for(; iterations<100*million; ++iterations)
    {
        int dir = dis(gen);
        x += dx[dir];
        y += dy[dir];
        if(y<0)
        {
            x=sx;
            y=sy;
        }
        else {
            if(is_chain[y][x]) {
                grid[y][x]++;
                x=sx;
                y=sy;
            }
        }
    }
    write_bmp_from_float("nplot.bmp",&grid[0][0],X,Y, 10.0f);

    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << iterations << " steps took: "
              << std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()
              << " seconds\n";
              

    return EXIT_SUCCESS;
}

void write_bmp_from_float(const std::string& filename, const uint64_t *g, const int w, const int h, const float gain)
{
    // g[w*h] is row-major float values, where w is the image width, h is image height
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
    uint8_t bgr[3];
    for(int i=0; i<h; i++)
    {
        for(int j=0;j<w;j++)
        {
            float val = g[(h-i-1)*w+j] * gain;
            uint8_t c = val>255.0f ? 255 : ( val < 0.0f ? 0 : static_cast<uint8_t>(val) );
            bgr[0]=c; bgr[1]=c; bgr[2]=c;
            output.write(reinterpret_cast<char*>(bgr),3);
        }
        output.write(bmppad,(4-(w*3)%4)%4);
    }
}