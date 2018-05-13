// Local:
#include "line.h"
#include "write_bmp.h"

// STL:
#include <chrono>
#include <cstdlib>
#include <random>

// stdlib:
#include <inttypes.h>

/*

Do a (vN) random walk from a point on the edge of an area. If crosses the x-axis then start again.
If hits the curved wall then accumulate there and start again.

We want to check whether all points on the curved path are hit with equal probability.

*/

double measured_llrr_radial[181]={1,1.003365683,1.005434206,1.00684812,1.008891366,1.009639134,1.011655588,1.013019178,1.013415966,1.015077452,1.015088911,1.015782549,1.017104369,1.017724254,1.018025931,1.01863815,1.018853558,1.019160225,1.018363306,1.018893894,1.019020727,1.018438889,1.018165667,1.017905322,1.01795829,1.017897196,1.017424775,1.01697323,1.016399528,1.016134151,1.01545964,1.013937315,1.013439788,1.013249811,1.0126546,1.01192265,1.009891885,1.00862017,1.009070278,1.007966787,1.006062515,1.005124431,1.004283637,1.00238423,1.001698137,0.999953577,0.998970169,0.997150073,0.99517768,0.994416698,0.991900596,0.990591538,0.989132123,0.987526396,0.985707773,0.983736874,0.982778741,0.980696062,0.97819741,0.975998023,0.973821432,0.972098209,0.969794589,0.968223639,0.965511762,0.962943434,0.960530732,0.958120856,0.955411144,0.952991577,0.951276487,0.948119362,0.944948681,0.942163503,0.939573058,0.937051695,0.934191237,0.93129918,0.929003235,0.926046635,0.922999469,0.920664074,0.917285857,0.914548705,0.910454689,0.907313058,0.904449197,0.901561445,0.897648132,0.894993999,0.890996785,0.887594006,0.884140304,0.880622457,0.877691534,0.873747577,0.87004501,0.866570462,0.863081448,0.859131555,0.855441084,0.852013404,0.847459846,0.842849611,0.839438505,0.835187881,0.83127485,0.827129654,0.82323314,0.819180158,0.815073726,0.810392608,0.806256616,0.801649763,0.797585621,0.793633881,0.788774884,0.784155841,0.779346138,0.774905471,0.769701179,0.765567576,0.760505902,0.755232914,0.750907613,0.745474729,0.740793865,0.736786463,0.731654344,0.726547236,0.721456641,0.71658515,0.711267974,0.705260737,0.700393437,0.694829043,0.688795591,0.684104817,0.678477012,0.673057947,0.667812916,0.661616174,0.656525974,0.650933877,0.645475948,0.63905359,0.633276457,0.627161331,0.620695223,0.614417207,0.608933012,0.602915841,0.596804142,0.590454916,0.583850584,0.576849233,0.57058555,0.564790333,0.55828307,0.551465788,0.544096769,0.536595451,0.529467577,0.523208731,0.51547781,0.507993506,0.500356672,0.494040225,0.485533208,0.476928658,0.468847889,0.460014456,0.452003577,0.4428568,0.433246901,0.422841808,0.411927728,0.400866763,0.388337786,0.372081302,0.349517685}; // [0,180] degrees from small indentation, measured after 1 trillion steps

void write_bmp_from_uint64(const std::string& filename, const uint64_t *g, const int w, const int h, const float gain);

bool inMoore(int x,int y,int tx,int ty) { return abs(x-tx)<=1 && abs(y-ty)<=1; }
bool onGrid(int x,int y,int X,int Y) { return x>=0 && x<X && y>=0 && y<Y; }
int dist2(int x,int y,int sx,int sy) { return (x-sx)*(x-sx)+(y-sy)*(y-sy); }

const float PI=3.14159265358979f;

int main()
{
    const int X = 100;
    const int Y = 40;
    int chain_idx[Y][X];
    bool path[Y][X];
    const int sx = X/2;
    const int sy = 0;
    int x = sx;
    int y = sy;
    const int N_DIRS = 4; // NESW
    const int dx[N_DIRS] = {0,1,0,-1};
    const int dy[N_DIRS] = {-1,0,1,0};
    const int dx8[8] = { 0,1,1,1,0,-1,-1,-1 };
    const int dy8[8] = { -1,-1,0,1,1,1,0,-1 };

    std::vector<uint64_t> impacts;

    // clear the arrays
    for (int fy = 0; fy < Y; fy++) {
        for (int fx = 0; fx < X; fx++) {
            chain_idx[fy][fx] = -1;
            path[fy][fx] = false;
        }
    }
    
    // initialize chain
    int ax = 0;
    int ay = 0;
    for(int theta=0;theta<=180;theta++)
    {
        const double rad = theta * PI / 180.0;
        const double r = X/3 * measured_llrr_radial[theta];
        int bx = (int)round(sx + r * cos(rad));
        int by = (int)round(sy + r * sin(rad));
        if(theta>0) {
            line(ax, ay, bx, by, [&](int x,int y) {
                if(chain_idx[y][x]==-1) {
                    chain_idx[y][x] = (int)impacts.size();
                    impacts.push_back(0);
                }
            });
        }
        ax = bx;
        ay = by;
    }

    // draw the chain for debugging
    {
        uint64_t grid[Y][X] = {0};
        grid[sy][sx] = 255;
        for(int fy=0;fy<Y;fy++) {
            for(int fx=0;fx<X;fx++) {
                if( chain_idx[fy][fx] > -1 )
                {
                    grid[fy][fx] = 255;
                }
            }
        }
        std::wcout << std::endl;
        write_bmp_from_uint64("shape_test.bmp",&grid[0][0],X,Y, 1.0f);
    }
    
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> random_move(0, N_DIRS - 1);
    
    auto t1 = std::chrono::high_resolution_clock::now();

    const uint64_t million = 1000000ul;
    const uint64_t billion = 1000*million;
    const uint64_t trillion = 1000*billion;
    uint64_t iterations = 0;
    for(; iterations<10*million; ++iterations)
    {
        int dir = random_move(gen);
        int new_x = x + dx[dir];
        int new_y = y + dy[dir];
        bool stuck = ( new_y>0 && ( path[new_y-1][new_x] || chain_idx[new_y-1][new_x]>=0 ) )
            && ( path[new_y+1][new_x] || chain_idx[new_y+1][new_x]>=0 )
            && ( path[new_y][new_x-1] || chain_idx[new_y][new_x-1]>=0 )
            && ( path[new_y][new_x+1] || chain_idx[new_y][new_x+1]>=0 );
        if(new_y<0 || stuck)
        {
            // cross the axis, start afresh
            x = sx;
            y = sy;
            // clear the path
            for (int fy = 0; fy < Y; fy++) {
                for (int fx = 0; fx < X; fx++) {
                    path[fy][fx] = false;
                }
            }
            continue;
        }
        /*if (!onGrid(new_x, new_y, X, Y)) // DEBUG
        {
            std::cerr << "Moved off grid" << std::endl;
            exit(EXIT_FAILURE);
        }*/
        // disallow the move if would step on our path
        if( path[new_y][new_x] )
            continue;
        int iChain = chain_idx[new_y][new_x];
        if( iChain >= 0) {
            // make a note of the impact but don't take the step
            impacts[iChain]++;
        }
        else {
            // take the step
            x = new_x;
            y = new_y;
            path[y][x] = true;
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << iterations << " steps took: "
              << std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()
              << " seconds\n";
    
    // output the impact counts to console
    for(uint64_t c : impacts) { printf("%" PRIu64 " ",c); }
    printf("\n");

    return EXIT_SUCCESS;
}

void write_bmp_from_uint64(const std::string& filename, const uint64_t *g, const int w, const int h, const float gain)
{
    write_bmp(filename,w,h,[&](int x,int y,char* bgr) {
        float val = g[(h-y-1)*w+x] * gain;
        uint8_t c = val>255.0f ? 255 : ( val < 0.0f ? 0 : static_cast<uint8_t>(val) );
        bgr[0]=c; bgr[1]=c; bgr[2]=c;
    });
}