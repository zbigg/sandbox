#include <iostream>

typedef unsigned long long NUM;

int main()
{
     const int N = 99;
     NUM s = 0;

     NUM ss = s;
     for(NUM i = 1; i <= N ; i++ ) {
         s += i;
         ss += i*i;
     }
     NUM s2 = s*s;
     std::cout << "N=" << N << "\n";
     std::cout << " square of sums  " << s2 << "\n";
     std::cout << " sums of squares " << ss << "\n";
     std::cout << " diff            " << (s2-ss) << "\n";
}
