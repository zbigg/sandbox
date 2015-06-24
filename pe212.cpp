#include <iostream>
#include <vector>
#include <cassert>
#include <cstdlib>

typedef unsigned long long bigint;


void lagged_fibinaccci_generator(std::vector<int>& v, int N)
{
    for( bigint k = 1 ; k <= std::min( 55, N ); ++k ) {
        const bigint k3 = k * k * k;
        v[k] = ( 100003 - (200003 * k ) + (300007 * k3 ) ) % 1000000;
    }
    for( size_t k = 56; k <= N; ++k ) {
        v[k] = ( v[k-24] + v[k-55] ) % 1000000;
    }
}

struct cuboid {
    int x,y,z;
    int dx,dy,dz;
};

std::ostream& operator<<(std::ostream& s, cuboid const& c)
{
    return s << "(" << c.x << "," << c.y << "," << c.z << ")(" << c.dx << "," << c.dy << "," << c.dz << ")";
}

#define my_assert(predicate, val) do { if( !(predicate) ) { std::cerr << "assert failed, " #val "=" << val << " at " << __LINE__ << std::endl; std::abort(); } } while(0)
bool mark( std::vector<bool>& space, cuboid c, int p )
{
    const bigint PN = 10400;
    const int pw = 10; // 10400 / 1040
    const int px0 = p * pw;
    const int px1 = px0 + pw;
    bool marked = false;
    for( int x = c.x; x < c.x + c.dx; x++ ) {
        if( x >= px1 )
            continue;
        if( x < px0 )
            continue;
        for( int y = c.y; y < c.y + c.dy; y++ ) {
            for( int z = c.z; z < c.z + c.dz; z++ ) {
                bigint bit_idx = ( (x - px0) * PN *PN ) + ( y * PN ) +  z;
                my_assert( bit_idx < space.size(), bit_idx );
                space[bit_idx] = 1;
                marked = true;
            }
        }
    }
    return marked;
}

int main()
{
     const int SN = 300000;
     const int CN = 50000;

     std::vector<int> lfs( SN + 1 );

     std::cout << "generating LFC, SN=" << SN << std::endl;
     lagged_fibinaccci_generator(lfs, SN);

     // for(int i = 1; i <= SN ; ++i ) {
     //    std::cout << " " << lfs[i];
     //}

     std::cout << "generating cuboids, CN=" << CN << std::endl;
     std::vector<cuboid> c( CN + 1 );
     for(int i = 1 ; i <= CN; ++i ) {
         c[i] = { lfs[ 6*i - 5 ] % 10000,
                  lfs[ 6*i - 4 ] % 10000,
                  lfs[ 6*i - 3 ] % 10000,
                  1 + lfs[ 6*i-2 ] % 399,
                  1 + lfs[ 6*i-1 ] % 399,
                  1 + lfs[ 6*i ] % 399 };
         assert( c[i].x < 10000 );
         assert( c[i].y < 10000 );
         assert( c[i].z < 10000 );
         assert( c[i].dx < 400 );
         assert( c[i].dy < 400 );
         assert( c[i].dz < 400 );
     }

     std::cout << "c1=" << c[1] << std::endl;
     std::cout << "c2=" << c[2] << std::endl;

     const bigint N = 10000+400;
     const bigint N3 = N*N*N;
     const bigint PS = N3 / 1040;

     std::cout << "PS=" << PS << std::endl;
     const bigint P = ( N3 / PS) + 1;
     bigint V = 0;
     for( int p = 0; p < P; p++ ) {
         std::cout << "p=" << p << "/" << P << "..." << std::endl;
         std::vector<bool> space(PS + 1, false);
         for( int ci = 1; ci <= CN; ci++ ) {
             if( mark( space, c[ci], p ) ) {
                 std::cout << "c[" << ci << "] marked" << std::endl;
             }
         }
         bigint pV = 0;
         for( int i = 0; i < PS; ++i ) {
             if( space[i] ) {
                 pV++;
             }
         }
         V += pV;
         std::cout << " pV=" << pV << ", v=" << V << "..." << std::endl;
     }
     std::cout << "result: V=" << V << std::endl;
}
