// Local:
#include "write_bmp.h"

// STL:
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>

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
int dist2(int x,int y,int sx,int sy) { return (x-sx)*(x-sx)+(y-sy)*(y-sy); }

int main()
{
    const int X = 100;
    const int Y = 40;
    int is_chain[Y][X];
    const int sx = X/2;
    const int sy = 0;
    int x = sx;
    int y = sy;
    const int N_DIRS = 4; // NESW
    const int dx[N_DIRS] = {0,1,0,-1};
    const int dy[N_DIRS] = {-1,0,1,0};
    const int dx8[8] = { 0,1,1,1,0,-1,-1,-1 };
    const int dy8[8] = { -1,-1,0,1,1,1,0,-1 };
    const int N_CHAIN_POINTS = 100;
    int chain[N_CHAIN_POINTS][2];
    uint64_t impacts[N_CHAIN_POINTS] = {0};

    for (int fy = 0; fy < Y; fy++) { for (int fx = 0; fx < X; fx++) is_chain[fy][fx] = -1; }
    
    // initialize chain to be a half square
    int cx = sx - N_CHAIN_POINTS / 4;
    int cy = sy;
    for(int i=0;i<N_CHAIN_POINTS;i++)
    {
        chain[i][0]=cx;
        chain[i][1]=cy;
        is_chain[cy][cx]=i;
        if(i>=3*N_CHAIN_POINTS/4-2) { cy -= 1; }
        else if(i>N_CHAIN_POINTS/4) { cx += 1; }
        else cy += 1;
    }
    
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> random_move(0, N_DIRS - 1);
    std::uniform_int_distribution<> random_move8(0, 7);
    std::uniform_int_distribution<> random_chain_point(0, N_CHAIN_POINTS-1);

    auto t1 = std::chrono::high_resolution_clock::now();
    
    const uint64_t million = 1000000ul;
    const uint64_t billion = 1000*million;
    const uint64_t trillion = 1000*billion;
    uint64_t iterations = 0;
    for(; iterations<billion; ++iterations)
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
            int iChain = is_chain[y][x];
            if(iChain>=0) {
                impacts[iChain]++;
                x=sx;
                y=sy;
            }
        }
        
        // tweak the chain every so often
        if(iterations>0 && (iterations % 10000 == 0)) {
            //for(int j=0;j<N_CHAIN_POINTS/2;j++) 
            {
                int iChainPt = random_chain_point(gen);
                int chain_mv = random_move8(gen);
                int old_cx = chain[iChainPt][0];
                int old_cy = chain[iChainPt][1];
                int new_cx = old_cx + dx8[chain_mv];
                int new_cy = old_cy + dy8[chain_mv];
                // move legal or not?
                if( onGrid(new_cx,new_cy,X,Y) && new_cx!=sx && new_cy!=sy && (is_chain[new_cy][new_cx]==-1)
                 && (iChainPt<=0 || inMoore(new_cx,new_cy,chain[iChainPt-1][0],chain[iChainPt-1][1]))
                 && (iChainPt>=N_CHAIN_POINTS-1 || inMoore(new_cx,new_cy,chain[iChainPt+1][0],chain[iChainPt+1][1]))
                 && (iChainPt>0 || (new_cy==old_cy && new_cx<sx))
                 && (iChainPt<N_CHAIN_POINTS-1 || (new_cy==old_cy && new_cx>sx)) ) {
                    // move beneficial or not?
                    int radial_delta = dist2(new_cx,new_cy,sx,sy+10)-dist2(old_cx,old_cy,sx,sy+10);
                    float neighbor_impact_average = ((iChainPt > 0 ? impacts[iChainPt - 1] : 0) + (iChainPt < N_CHAIN_POINTS - 1 ? impacts[iChainPt + 1] : 0)) / ((iChainPt>0&&iChainPt<N_CHAIN_POINTS-1)?2.0f:1.0f);
                    float impact_laplacian = impacts[iChainPt] - neighbor_impact_average;
                    if( ( radial_delta > 0 && impact_laplacian > 0 ) || ( radial_delta < 0 && impact_laplacian < 0 ) )
                    {
                        // accept the move
                        is_chain[old_cy][old_cx]=-1;
                        is_chain[new_cy][new_cx]=iChainPt;
                        chain[iChainPt][0]=new_cx;
                        chain[iChainPt][1]=new_cy;
                        impacts[iChainPt] = static_cast<uint64_t>(neighbor_impact_average);
                    }
                }
            }
            x = sx;
            y = sy;
            //std::fill_n(&grid[0][0],X*Y,0);
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << iterations << " steps took: "
              << std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()
              << " seconds\n";

    // draw the chain onto the image
    uint64_t grid[Y][X] = {0};
    grid[sy][sx] = 255;
    //grid[sy+1][sx] = 255;
    for(int iChain=0;iChain<N_CHAIN_POINTS;iChain++) {
        grid[chain[iChain][1]][chain[iChain][0]] = 255;
        std::cout << impacts[iChain] << " ";
    }
    std::wcout << std::endl;
    write_bmp_from_uint64("chain_shape.bmp",&grid[0][0],X,Y, 1.0f);

    return EXIT_SUCCESS;
}

void write_bmp_from_uint64(const std::string& filename, const uint64_t *g, const int w, const int h, const float gain)
{
    write_bmp(filename,w,h,[&](int x,int y,char bgr[3]) {
        float val = g[y*w+x] * gain;
        uint8_t c = val>255.0f ? 255 : ( val < 0.0f ? 0 : static_cast<uint8_t>(val) );
        bgr[0]=c; bgr[1]=c; bgr[2]=c;
    });
}