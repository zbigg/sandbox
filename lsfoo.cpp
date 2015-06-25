#include "tinfra/fs.h"
#include "tinfra/tstring.h"

using tinfra::tstring;
using std::vector;

typedef vector<std::string> plist;
typedef vector<tstring> vlist;

struct  task {
     vector<vlist> statements;
};

void parse(std::string const& input, task& task)
{
    const char* first = &input[0];
    const char* c = first;
    const char* last = &input[input.size()-1];
    enum {
        NORMAL,
        WORD,
        IN_QUOTES,
    } state;
    while( c != last ) {
        switch( state ) {
        case NORMAL:
            if( isspace(*c) ) {
                c++;
                continue;
            } if( *c == '"' ) {
                state = IN_QUOTES;
                c++;
                first_char = 
            }
                
        case WORD:
        case IN_QUOTES:
        }
    }
}
int main()
{
}