// Local:
#include "defs.h"
#include "line.h"
#include "write_bmp.h"

// C++ Standard Library:
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cinttypes>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

/*

Do a (vN) random walk from a point on the edge of an area. If crosses the x-axis then start again.
If hits the curved wall then accumulate there and start again.

We want to check whether all points on the curved path are hit with equal probability.

*/

const double measured_llrr_radial[181]={1,1.003365683,1.005434206,1.00684812,1.008891366,1.009639134,1.011655588,1.013019178,1.013415966,1.015077452,1.015088911,1.015782549,1.017104369,1.017724254,1.018025931,1.01863815,1.018853558,1.019160225,1.018363306,1.018893894,1.019020727,1.018438889,1.018165667,1.017905322,1.01795829,1.017897196,1.017424775,1.01697323,1.016399528,1.016134151,1.01545964,1.013937315,1.013439788,1.013249811,1.0126546,1.01192265,1.009891885,1.00862017,1.009070278,1.007966787,1.006062515,1.005124431,1.004283637,1.00238423,1.001698137,0.999953577,0.998970169,0.997150073,0.99517768,0.994416698,0.991900596,0.990591538,0.989132123,0.987526396,0.985707773,0.983736874,0.982778741,0.980696062,0.97819741,0.975998023,0.973821432,0.972098209,0.969794589,0.968223639,0.965511762,0.962943434,0.960530732,0.958120856,0.955411144,0.952991577,0.951276487,0.948119362,0.944948681,0.942163503,0.939573058,0.937051695,0.934191237,0.93129918,0.929003235,0.926046635,0.922999469,0.920664074,0.917285857,0.914548705,0.910454689,0.907313058,0.904449197,0.901561445,0.897648132,0.894993999,0.890996785,0.887594006,0.884140304,0.880622457,0.877691534,0.873747577,0.87004501,0.866570462,0.863081448,0.859131555,0.855441084,0.852013404,0.847459846,0.842849611,0.839438505,0.835187881,0.83127485,0.827129654,0.82323314,0.819180158,0.815073726,0.810392608,0.806256616,0.801649763,0.797585621,0.793633881,0.788774884,0.784155841,0.779346138,0.774905471,0.769701179,0.765567576,0.760505902,0.755232914,0.750907613,0.745474729,0.740793865,0.736786463,0.731654344,0.726547236,0.721456641,0.71658515,0.711267974,0.705260737,0.700393437,0.694829043,0.688795591,0.684104817,0.678477012,0.673057947,0.667812916,0.661616174,0.656525974,0.650933877,0.645475948,0.63905359,0.633276457,0.627161331,0.620695223,0.614417207,0.608933012,0.602915841,0.596804142,0.590454916,0.583850584,0.576849233,0.57058555,0.564790333,0.55828307,0.551465788,0.544096769,0.536595451,0.529467577,0.523208731,0.51547781,0.507993506,0.500356672,0.494040225,0.485533208,0.476928658,0.468847889,0.460014456,0.452003577,0.4428568,0.433246901,0.422841808,0.411927728,0.400866763,0.388337786,0.372081302,0.349517685}; // [0,180] degrees from small indentation, measured after 1 trillion steps using llrr.cpp

const double measured_random_walk_radial[181]={1,1.144048402,1.268977768,1.355015768,1.432079075,1.479793297,1.525271588,1.55423251,1.590896719,1.621708065,1.646383837,1.665225882,1.684681529,1.704469732,1.721676977,1.736309592,1.751267846,1.763710444,1.773660405,1.783414891,1.795142355,1.806374321,1.817120841,1.827360284,1.837069607,1.846248419,1.854926749,1.863155028,1.870983719,1.8784446,1.885544142,1.892272931,1.898626711,1.904627791,1.910333845,1.915826199,1.921180375,1.926432496,1.931559426,1.936484456,1.941107072,1.945342758,1.949154752,1.952565721,1.95564876,1.958506293,1.961247901,1.963974198,1.966767708,1.969687425,1.972763357,1.975990278,1.979323837,1.982684182,1.985970443,1.989084244,1.991954765,1.994555668,1.996907219,1.999063529,2.001091019,2.00304664,2.00496284,2.006842666,2.008665074,2.010398128,2.012015691,2.013511741,2.01490642,2.016240882,2.017562999,2.018910548,2.020299579,2.021722418,2.023154431,2.024564661,2.025925092,2.027215994,2.028428037,2.029563148,2.030634848,2.0316667,2.032686713,2.033717252,2.034763229,2.035803707,2.036791401,2.037661037,2.038343268,2.038778766,2.038928444,2.038778766,2.038343268,2.037661037,2.036791401,2.035803707,2.034763229,2.033717252,2.032686713,2.0316667,2.030634848,2.029563148,2.028428037,2.027215994,2.025925092,2.024564661,2.023154431,2.021722418,2.020299579,2.018910548,2.017562999,2.016240882,2.01490642,2.013511741,2.012015691,2.010398128,2.008665074,2.006842666,2.00496284,2.00304664,2.001091019,1.999063529,1.996907219,1.994555668,1.991954765,1.989084244,1.985970443,1.982684182,1.979323837,1.975990278,1.972763357,1.969687425,1.966767708,1.963974198,1.961247901,1.958506293,1.95564876,1.952565721,1.949154752,1.945342758,1.941107072,1.936484456,1.931559426,1.926432496,1.921180375,1.915826199,1.910333845,1.904627791,1.898626711,1.892272931,1.885544142,1.8784446,1.870983719,1.863155028,1.854926749,1.846248419,1.837069607,1.827360284,1.817120841,1.806374321,1.795142355,1.783414891,1.773660405,1.763710444,1.751267846,1.736309592,1.721676977,1.704469732,1.684681529,1.665225882,1.646383837,1.621708065,1.590896719,1.55423251,1.525271588,1.479793297,1.432079075,1.355015768,1.268977768,1.144048402,1}; // [0,180] degrees, measured using walk_plot.cpp

const double optimized_radial_profile1[181]={1.119892,1.204358,1.243752,1.275329,1.316589,1.339737,1.367579,1.381811,1.406476,1.427300,1.444576,1.458801,1.476194,1.496018,1.513411,1.530104,1.547660,1.564191,1.578858,1.593857,1.611609,1.628264,1.643955,1.659654,1.674888,1.690567,1.705922,1.721633,1.736862,1.752180,1.766416,1.781353,1.795995,1.810730,1.825225,1.839432,1.853230,1.866803,1.879609,1.893324,1.907331,1.920541,1.934923,1.948833,1.961966,1.975785,1.989534,2.002617,2.014701,2.028075,2.041010,2.054182,2.067259,2.079669,2.091302,2.101426,2.109606,2.116288,2.121368,2.125062,2.128130,2.130261,2.132611,2.134659,2.136631,2.138887,2.140769,2.142546,2.143931,2.145152,2.145871,2.147948,2.148739,2.150101,2.151241,2.152093,2.152551,2.153084,2.153603,2.154324,2.155067,2.155732,2.156541,2.156865,2.157557,2.157451,2.157466,2.157482,2.157461,2.157418,2.157461,2.157418,2.157461,2.157482,2.157466,2.157451,2.157557,2.156865,2.156541,2.155732,2.155067,2.154324,2.153603,2.153084,2.152551,2.152093,2.151241,2.150101,2.148739,2.147948,2.145871,2.145152,2.143931,2.142546,2.140769,2.138887,2.136631,2.134659,2.132611,2.130261,2.128130,2.125062,2.121368,2.116288,2.109606,2.101426,2.091302,2.079669,2.067259,2.054182,2.041010,2.028075,2.014701,2.002617,1.989534,1.975785,1.961966,1.948833,1.934923,1.920541,1.907331,1.893324,1.879609,1.866803,1.853230,1.839432,1.825225,1.810730,1.795995,1.781353,1.766416,1.752180,1.736862,1.721633,1.705922,1.690567,1.674888,1.659654,1.643955,1.628264,1.611609,1.593857,1.578858,1.564191,1.547660,1.530104,1.513411,1.496018,1.476194,1.458801,1.444576,1.427300,1.406476,1.381811,1.367579,1.339737,1.316589,1.275329,1.243752,1.204358,1.119892}; // [0,180] degrees, output of this program

bool inMoore(int x,int y,int tx,int ty) { return abs(x-tx)<=1 && abs(y-ty)<=1; }
bool onGrid(int x,int y,int X,int Y) { return x>=0 && x<X && y>=0 && y<Y; }
int dist2(int x,int y,int sx,int sy) { return (x-sx)*(x-sx)+(y-sy)*(y-sy); }
bool onLeft(int x,int y,int x1,int y1,int x2,int y2) {
    if (y1 == y2 && x1 == x2) { return false; } // zero-length line
    float perp_x = float(y1-y2); // -dy
    float perp_y = float(x2-x1); // dx
    float lp = hypot(perp_x,perp_y);
    perp_x /= lp;
    perp_y /= lp;
    float dot = perp_x * (x-x1) + perp_y * (y-y1);
    return dot > 2;
}

int main()
{
    const int X = 1000;
    const int Y = 600;
    int chain_idx[Y][X];
    const int sx = X/2;
    const int sy = 0;
    int x = sx;
    int y = sy;
    const int N_DIRS = 4; // NESW
    const int dx[N_DIRS] = {0,1,0,-1};
    const int dy[N_DIRS] = {-1,0,1,0};
    const int dx8[8] = { 0,1,1,1,0,-1,-1,-1 };
    const int dy8[8] = { -1,-1,0,1,1,1,0,-1 };

    struct Impact { int x, y; double n; };
    std::vector<Impact> impacts;

    std::vector<double> radial(std::begin(optimized_radial_profile1), std::end(optimized_radial_profile1));

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> random_move(0, N_DIRS - 1);
    
    const bool for_cardioid = true;
    for (int iAdjustment = 0; ; iAdjustment++)
    {
        // initialize chain from radial
        for (int fy = 0; fy < Y; fy++) {
            for (int fx = 0; fx < X; fx++) {
                chain_idx[fy][fx] = -1;
            }
        }
        int ax = 0;
        int ay = 0;
        impacts.clear();
        for (int theta = 0; theta <= 180; theta++)
        {
            const double rad = theta * PI / 180.0;
            const double r = X / 6 * radial[theta];
            int bx = std::clamp((int)round(sx + r * cos(rad)), 0, X - 1);
            int by = std::clamp((int)round(sy + r * sin(rad)), 0, Y - 1);
            if (theta > 0) {
                line(ax, ay, bx, by, [&](int x, int y) {
                    if (chain_idx[y][x] == -1) {
                        chain_idx[y][x] = (int)impacts.size();
                        impacts.push_back(Impact({ x, y, 0 }) );
                    }
                });
            }
            ax = bx;
            ay = by;
        }

        // draw the chain for debugging
        std::ostringstream oss;
        oss << "shape_test_chain2_" << std::setfill('0') << std::setw(4) << iAdjustment << ".bmp";
        write_bmp(oss.str(), X, Y, [&](int x, int y, unsigned char bgr[3]) {
            if( (x==sx && y==sy) || (chain_idx[y][x] > -1) ) { bgr[0] = bgr[1] = bgr[2] = 255; }
            else { bgr[0] = bgr[1] = bgr[2] = 0; }
        });
        std::cout << "Wrote " << oss.str() << std::endl;
        
        if(iAdjustment==20) break;

        auto t1 = std::chrono::high_resolution_clock::now();

        int iChain_last_impact = (int)impacts.size();

        uint64_t num_impacts = 0;
        uint64_t iterations = 0;
        const uint64_t expected_impacts = 100;
        x = sx;
        y = sy;
        for (; iterations < 10*billion && num_impacts < expected_impacts * impacts.size(); ++iterations)
        {
            int dir = random_move(gen);
            int new_x = x + dx[dir];
            int new_y = y + dy[dir];
            if (new_y < 0)
            {
                // start afresh
                x = sx;
                y = sy;
                if(for_cardioid) {
                    iChain_last_impact = (int)impacts.size();
                }
                continue;
            }
            int iChain = chain_idx[new_y][new_x];
            if (iChain >= 0) {
                if(iChain>=iChain_last_impact) {
                    // hit the curved wall to the left of a previous impact,
                    // ignore this move and continue
                    continue;
                }
                // make a note of the impact
                impacts[iChain].n++;
                num_impacts++;
                
                if(for_cardioid) {
                    // record the new impact point and continue from here without taking the step
                    iChain_last_impact = iChain;
                }
                else {
                    // start afresh
                    x = sx;
                    y = sy;
                    continue;
                }
            }
            else {
                // take the step
                x = new_x;
                y = new_y;
            }

        }

        // output the impact counts to console
        printf("\nAfter %d adjustments:\n",iAdjustment);
        for(const Impact& i : impacts) { printf("%f ",i.n); }
        printf("\n\n");

        // smooth the impacts arrays
        {
            for (size_t i = 0; i < 1000; i++) {
                std::vector<Impact> impacts2(impacts);
                for (size_t j = 0; j < impacts.size(); j++) {
                    double sum = impacts2[j].n;
                    int n = 1;
                    if (j > 0) { sum += impacts2[j - 1].n; n++; }
                    if (j < impacts.size() - 1) { sum += impacts2[j + 1].n; n++; }
                    impacts[j].n = sum / n;
                }
            }
        }

        // output the impact counts to console
        printf("\nSmoothed:\n");
        for(const Impact& i : impacts) { printf("%f ",i.n); }
        printf("\n\n");

        // adjust the radial profile by the impacts
        {
            struct Acc { int n; double val; };
            std::vector<Acc> acc;
            for (int theta = 0; theta <= 180; theta++)
            {
                acc.push_back( Acc({0,0}) );
            }
            for(const Impact& i : impacts) {
                const double theta = 180.0 * atan2(i.y-sy,i.x-sx) / PI;
                const int itheta = (int)round(theta);
                acc[itheta].val += i.n;
                acc[itheta].n++;
                if(!for_cardioid) {
                    // make symmetrical
                    acc[180-itheta].val += i.n;
                    acc[180-itheta].n++;
                }
            }
            for(int theta=0;theta<=180;theta++)
            {
                if (acc[theta].n == 0) { printf("No impacts at degree %d!\n", theta); exit(EXIT_FAILURE); }
                const double impacts_here = acc[theta].val / acc[theta].n;
                const double delta_r = ( 0.1 * ( impacts_here - expected_impacts ) ) / expected_impacts; // more impacts -> move outwards
                radial[theta] += delta_r;
            }
        }

        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << iterations << " steps took: "
            << std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()
            << " seconds\n";
    }

    // output the final radial profile to console
    printf("\nFinal radial profile:\n");
    for(double r : radial) { printf("%f ",r); }
    printf("\n\n");

    return EXIT_SUCCESS;
}
