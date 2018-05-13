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

