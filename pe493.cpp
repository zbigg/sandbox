#include "xbmath/xbmath.h"
#include "tinfra/fmt.h"
#include "tinfra/stream.h"
#include <algorithm>
#include <set>
#include <vector>
#include <sstream>
#include <iomanip>

template <typename T>
std::ostream& operator<<(std::ostream& s, std::vector<T> const& v)
{
    s << "[";
    for(size_t i = 0; i < v.size(); ++i ) {
        if( i > 0 ) s << ", ";
        s << v[i];
    }
    return s << "]";
}

#ifdef RRR
using xbmath::rational;
#else
typedef double rational;
#endif
using tinfra::tprintf;

std::string S(int i)
{
    return std::string(1, "XYZabcdef"[i]);
}

#ifdef RRR
std::string R(rational r)
{
    if( r.q.is_one() ) {
        return tinfra::tsprintf( "%s", r.p );
    } else {
        return tinfra::tsprintf( "%s/%s", r.p, r.q );
    }
}
#else
#define R(n) (n)

#endif

struct state {
    std::vector<int> sym_stack;
    std::vector<double> probabilities;
} GTS;

rational calculate_ev()
{
    rational r = 0;
    for( unsigned i = 1; i < GTS.probabilities.size(); ++i  ) {
        r += i * GTS.probabilities[i];
    }
    return r;
}

//#define FOO
unsigned long long ttt = 0;
void gen_tree(int depth, int level, unsigned long long weight_left, std::vector<int>& syms, int bucket, rational& PS, rational parent_prop = 1.0)
{
#ifdef FOO
    std::string indent = std::string(level*2, ' ');
#endif
    // tprintf(tinfra::out, "%sgen_tree d=%s l=%s s=%s\r\n", indent, depth, level, 1);
    size_t consumed_symbols = 0;
    int    new_symols_weight = 0;
    size_t    allowed_new_symbol = 999999999;
    for( size_t i = 0; i < syms.size(); i++ ) {
        if( syms[i] != bucket ) {
            consumed_symbols++;
        } else {
            new_symols_weight += bucket;
            if( allowed_new_symbol == 999999999 ) {
                allowed_new_symbol = i;
            }
        }
    }
    const size_t branch_count = std::min(syms.size(), consumed_symbols+1);
    // tprintf(tinfra::out, "    %s .. cs=%s bc=%s\r\n", indent,consumed_symbols, branch_count);
    for( size_t is = 0, ib = 0; is < syms.size() && ib < branch_count; ++is ) {
        if( syms[is] == 0 ) {
            continue;
        }
        if( syms[is] == bucket && is != allowed_new_symbol ) {
            continue;
        }
#ifdef RRR
        rational p = rational( is == allowed_new_symbol ? new_symols_weight : syms[is], weight_left );
#else
        rational p = rational( is == allowed_new_symbol ? new_symols_weight : syms[is] ) / rational (weight_left );
#endif

        rational P = parent_prop * p;
#ifdef RRR
        P.shrink();
#endif
        GTS.sym_stack[level] = is;
#ifdef FOO
        if( level < 10 ) {
            tprintf(tinfra::out, "%s[%s] left=%s ib=%s p=%s L=%s [%s]\r\n", indent, S(is), syms[is]-1, ib, R(p), level, ttt);
        }
#endif
        if( level < depth-1 ) {
            // std::vector<int> child_syms(syms);
            syms[is] -= 1;
            gen_tree(depth, level+1, weight_left-1, syms, bucket, PS, P);
            syms[is] += 1;
        } else {
            ttt++;
            int v = 0;
            int m[30] = {0};
#ifdef BAR
            std::ostringstream buf;
#endif
            for( int i = 0; i < level+1; i++ ) {
                int s = GTS.sym_stack[i];
#ifdef BAR
                buf << S(s) << " ";
#endif
                if( !m[s] ) {
                    v++;
                    m[s] = 1;
                }
            }
#ifdef BAR
            buf << "P=" << std::setprecision(12) << R(P) << " V=" << v << "\r\n";
            tinfra::out.write(buf.str());
#endif
#define SPAM
            GTS.probabilities[v] += P;
            PS += P;
#ifdef SPAM
            if ( ttt % 100000000 == 0 ) {
                rational ev = calculate_ev();
                std::cout << ttt << std::setprecision(12) << " EV=" << R(ev) << " PS=" << R(PS) << std::endl;
            }
#endif
        }
        ib++;
    }
}

// typedef float rational;
int main()
{
    const int NSYMS = 7;
    const int BUCKET = 10;
    const int DEPTH = 16;
    GTS.sym_stack.resize(DEPTH);
    GTS.probabilities.resize(NSYMS+2);
    std::vector<int> syms(NSYMS, BUCKET);
    syms[0] -= 1;
    GTS.sym_stack[0] = 0;
#ifdef FOO
    tprintf(tinfra::out, "[X] left=%s ib=%s p=%s L=0\r\n", BUCKET-1,"n/a", R(1.0));
#endif
    unsigned long long weight = BUCKET * NSYMS;
    rational PS = 0;
    gen_tree(DEPTH, 1, weight-1, syms, BUCKET, PS);
    rational ev = calculate_ev();
#ifdef RRR
    PS.shrink();
    ev.shrink();
#endif
    tprintf(tinfra::out, "NSYMS=%s\r\n", NSYMS);
    tprintf(tinfra::out, "BUCKET=%s\r\n", BUCKET);
    tprintf(tinfra::out, "DEPTH=%s\r\n", DEPTH);
    tprintf(tinfra::out, "PS=%s\r\n", R(PS));
    tprintf(tinfra::out, "EV=%s (%.12s)\r\n", R(ev), ev);
}
