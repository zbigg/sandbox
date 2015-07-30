#include "tinfra/fs.h"
#include "tinfra/tstring.h"

struct tokenizer_iterator {
    const char* p;
    const char* end;

    int line_number;
    operator const char*() { return p }
    char operator*() { return *p; }

    tokenizer_iterator& operator++() {
        if( *p == '\n' ) {
            line_number++;
            start_of_line = true;
        } else {
        }
        p++;
    }
    tokenizer_iterator operator++(int) { tokenizer_iterator tmp(*this); ++(*this); return tmp; }
};

struct token {
    int t;
    tinfra::tstring s;
};
struct token_sink {
    std::vector<token> tokens;
};

bool operator<(tokenizer_iterator t, const char* c)
{
    return (const char*)t < c;
}

bool operator==(tokenizer_iterator t, const char* c)
{
    return (const char*)t == c;
}

tokenizer_iterator hash_comment(tokenizer_iterator c)
{
    while( c.valid() &&  *c == '#' ) {
        ++c;
        while(  c.valid() && *c != '\n' ) {
            ++c;
        }
        ++c;
    }
    return c;
}

tokenizer_iterator inifile_section(tokenizer_iterator c, token_sink e)
{
    if( c.valid() && c.start_of_line && *c == '[' ) {
        ++c;
        const char* name_start = c;
        while( c.valid() && *c != ']' ) {
            ++c;
        }
        if( c.valid() && *c == ']' ) {
            const char* name_end = c;
            token_emit(e, TOKEN_INIFILE_SECTION, name_start, name_end );
            ++c;
        } else {
            abort();
        }
    }
    return c;
}

tokenizer_iterator inifile_entry_whitespace(tokenizer_iterator c)
{
    while( c.valid() &&  std::isspace( *c  ) {
        ++c;
    }
}

tokenizer_iterator inifile_simple_ascii_word(tokenizer_iterator c, token_sink e)
{
    if( c.valid() && std::isalpha( *c  ) {
        const char* name_start = c;
        ++c;
        while( c.valid() && std::isalnum(*c ) ) {
            ++c;
        }
        if( c.valid() ) {
            const char* name_end = c;
            token_emit(e, TOKEN_ASCII_WORD, name_start, name_end );
        }
        ++c;
    }
}

tokenizer_iterator inifile_entry(tokenizer_iterator c, token_sink e)
{
    c = inifile_entry_whitespace(c);
    c = inifile_ascii_word(c, local);
    c = inifile_entry_whitespace(c);
    c = inifile_constant(c, "=", local);
    c = inifile_entry_whitespace(c);
    c = inifile_anything_endline(c, local);
    return c;
}

int main()
{
    token_sink sink;
    tokenizer<hash_comment, inifile_section, inifile_section> t(sink);
    while ( t.parse() ) {
        if( sink.contains( TOKEN_INIFILE_SECTION ) ) {
            // consume inifile section
            tstring section_name = sink[-1];
        } else if( t.contains( TOKEN_ASCII_WORD, TOKEN_EQUALS, TOKEN_INIFILE_RAW_VALUE ) ) {
            tstring key = sink[-1];
            tstring value = sink[-3];
        }
    }
}

