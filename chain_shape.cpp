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

void write_bmp_from_uint64(const std::string& filename, const uint64_t *g, const int w, const int h, const float gain);

bool inMoore(int x,int y,int tx,int ty) { return abs(x-tx)<=1 && abs(y-ty)<=1; }
bool onGrid(int x,int y,int X,int Y) { return x>=0 && x<X && y>=0 && y<Y; }

int main()
{
    const int X = 100;
    const int Y = 40;
    uint64_t grid[Y][X] = {0};
    bool is_chain[Y][X] = {false};
    const int sx = X/2;
    const int sy = 0;
    int x = sx;
    int y = sy;
    const int N_DIRS = 4; // NESW
    const int dx[N_DIRS] = {0,1,0,-1};
    const int dy[N_DIRS] = {-1,0,1,0};
    const int N_CHAIN_POINTS = 100;
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
    std::uniform_int_distribution<> random_move(0, N_DIRS-1);
    std::uniform_int_distribution<> random_chain_point(0, N_CHAIN_POINTS-1);

    auto t1 = std::chrono::high_resolution_clock::now();
    
    const uint64_t million = 1000000ul;
    const uint64_t billion = 1000*million;
    const uint64_t trillion = 1000*billion;
    uint64_t iterations = 0;
    for(; iterations<100*billion; ++iterations)
    {
        int dir = random_move(gen);
        x += dx[dir];
        y += dy[dir];
        if(y<0)
        {
            x=sx;
            y=sy;
        }
        else {
            /*if (!onGrid(x, y, X, Y)) {
                std::cerr << "Moved off grid:" << x << " " << y << std::endl;
                exit(EXIT_FAILURE);
            }*/
            if(is_chain[y][x]) {
                grid[y][x]++;
                x=sx;
                y=sy;
            }
        }
        
        // tweak the chain every so often
        if(iterations % million == 0) {
            for(int j=0;j<N_CHAIN_POINTS/2;j++) {
                int iChainPt = random_chain_point(gen);
                int chain_mv = random_move(gen);
                int old_cx = chain[iChainPt][0];
                int old_cy = chain[iChainPt][1];
                int new_cx = old_cx + dx[chain_mv];
                int new_cy = old_cy + dy[chain_mv];
                // move legal or not?
                if( onGrid(new_cx,new_cy,X,Y) && new_cx!=sx && new_cy!=sy && !is_chain[new_cy][new_cx]
                 && (iChainPt<=0 || inMoore(new_cx,new_cy,chain[iChainPt-1][0],chain[iChainPt-1][1]))
                 && (iChainPt>=N_CHAIN_POINTS-1 || inMoore(new_cx,new_cy,chain[iChainPt+1][0],chain[iChainPt+1][1]))
                 && (iChainPt>0 || (new_cy==old_cy && new_cx<sx))
                 && (iChainPt<N_CHAIN_POINTS-1 || (new_cy==old_cy && new_cx>sx)) ) {
                    // accept the move
                    is_chain[old_cy][old_cx]=false;
                    is_chain[new_cy][new_cx]=true;
                    chain[iChainPt][0]=new_cx;
                    chain[iChainPt][1]=new_cy;
                }
            }
            memset(&grid[0][0],0,X*Y*sizeof(uint64_t));
            x = sx;
            y = sy;
            //std::fill_n(&grid[0][0],X*Y,0);
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << iterations << " steps took: "
              << std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()
              << " seconds\n";
              
    write_bmp_from_uint64("nplot.bmp",&grid[0][0],X,Y, 100.0f);

    return EXIT_SUCCESS;
}

void write_bmp_from_uint64(const std::string& filename, const uint64_t *g, const int w, const int h, const float gain)
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