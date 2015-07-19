#include "tinfra/fmt.h"
#include "tinfra/stream.h"

using tinfra::tprintf;

double still_j(int j, int p, int n, int k)
{
	assert( n != p );
	return (double)( (j * k ) - p ) / ( n - p );
}

void gen_tree(int j, int p, int n, int k, int depth, double parent_prop, double* EV, double* PS)
{
	// we are at level p, and we already choosen 
	// j colors
	tprintf(tinfra::out, "(j=%s, p=%s) pp=%0.10s\n", j, p, parent_prop);
	if( p == depth ) {
		*PS += parent_prop;
		tprintf(tinfra::out, "EV[%s] +=%s at j=%s p=%s ps=%s\n", j, parent_prop, j, p, *PS);
		EV[j] += parent_prop;
	} else {
		double P = still_j(j, p, n, k);
		double PN = 1-P;
		if( P != 0.0 ) {
			gen_tree( j, p+1, n, k, depth, parent_prop * P, EV, PS );
		}
		if ( j <= k ) {
			gen_tree( j + 1, p+1, n, k, depth, parent_prop * PN, EV, PS );
		}
	}
}

int main(int argc, char** argv)
{
    const int N = atoi(argv[1]);
    const int K = atoi(argv[2]);
    const int DEPTH = atoi(argv[3]);
    
    double EV[100] = {0.0};
    double PS = 0.0;
    gen_tree( 0, 0, N, K, DEPTH, 1.0, EV, &PS );

    tprintf(tinfra::out, "N=%s K=%s DEPTH=%s\n", N, K, DEPTH);
    double ev = 0.0;
    for(int i = 0; i < DEPTH+1; ++i ) {
       tprintf(tinfra::out, "EV[%s]=%0.10s\n", i, EV[i]);
       ev += EV[i] * i;
    }
    tprintf(tinfra::out, "ev=%0.10s\n", ev);
    tprintf(tinfra::out, "PS=%0.10s\n", PS);
}