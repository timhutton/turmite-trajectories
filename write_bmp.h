// C++ Standard Library:
#include <fstream>
#include <functional>
#include <string>

void write_bmp(const std::string& filename, const int w, const int h, std::function<void(int x,int y,unsigned char bgr[3])> bgr_func)
{
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
    unsigned char bgr[3];
    for(int y = h-1; y >= 0; y--)
    {
        for(int x = 0; x < w; x++)
        {
            bgr_func(x,y,bgr);
            output.write(reinterpret_cast<char*>(bgr),3);
        }
        output.write(bmppad,(4-(w*3)%4)%4);
    }
}
