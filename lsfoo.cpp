
// lsfoo - file list generator
//
//   generates many file lists from several source trees
//   each of list gather different set of files, kinda find | awk 'COMPLICATED SCRIPT' on steroids
//
//  rules example:
//    sources-foo include-glob     src/*.{cpp|cc|C|c++|c}
//                                 # equivalent to src/*.cpp src/*.cc src/*.C ...
//    sources-foo include-regexp   src/.*.(cpp|cc|C|c++|c)$
//
//    sources-foo file             sources-foo.xml
//    sources-foo format           makefile-dep sources-foo

//    sources-foo output-format    text
//    sources-foo output-format    xml [root] [item]
//    sources-foo output-format    json

//    # globs have to be at bottom, because they actively affect 
//    # already declared lists
//
//    *   exclude-glob .git
//                  -- *~



#include "tinfra/fs.h"
#include "tinfra/tstring.h"

using tinfra::tstring;
using std::vector;

void consume_whitespace(tstring& line)
{
    tstring::iterator c = line.begin();
    while( c != line.end() ) {
        if( !isspace(*c) ) {
            break;
        }
        c++;
    }
    if( c != line.begin() ) {
        line = line.substr( c - line.begin() );
    }
}

void strip_endline( tstring& line )
{
    if( line.size() == 0 )
        return;

    const auto size = line.size();
    if( line[size - 1] == '\n' ) {
        if( size > 1 && line[size - 2] == '\r' ) {
            line = line.substr(0, size - 2);
        } else {
            line = line.substr(0, size - 1);
        }
    }
}

tstring rest(tstring& line)
{
    consume_whitespace( line );
    tstring result = line;
    strip_endline( result );
    line = "";
}

tstring next_word(tstring& line)
{
    consume_whitespace( line );
    tstring::iterator c = line.begin();
    while( c != line.end() ) {
        if( !isspace(*c) ) {
            c++;
            continue;
        } else {
            break;
        }
    }
    tstring result = line.substr( 0, c - line.begin() );
    line = line.substr( c - line.begin() );
    return result;
}

struct list_definition {
    
};

std::map<tstring, list_definition> lists;

void parse(tstring line)
{
    tstring list_specifier = next_word(line);
    if( list_specifier.empty() ) {
        return;
    }
    if( list_specifier == '*' ) {
    }
}
int main()
{
}