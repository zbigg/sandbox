#ifndef json_mo_parser_h_included
#define json_mo_parser_h_included

#include "tinfra/json.h"
#include "tinfra/lex.h"
#include "tinfra/fmt.h"
#include "tinfra/trace.h"
#include "tinfra/mo.h"

#include <vector>
#include <string>

// namespace tinfra {

extern tinfra::module_tracer json_mo_parser_tracer;

class json_mo_parser {
    tinfra::json_lexer& lexer;

    tinfra::json_token  current;
    bool        finished;
    std::vector<std::string> name_stack;

public:
    json_mo_parser(tinfra::json_lexer& lexer);

    // MO visitor contract
    template <typename S, typename T>
    void record(S sym, T& v) 
    {
        if( !match_name(sym) || finished ) {
            return;
        }
        expect(tinfra::json_token::OBJECT_BEGIN);
        next_or_fail("expected } or \"key\":value when parsing object but EOF found");

        if( current.type == tinfra::json_token::OBJECT_END ) {
            // short circuit for {}
            next();
            return;
        }

        while( true ) {
            // 'foo'
            expect(tinfra::json_token::STRING);
            std::string key(current.value.str());
            next();
            // 'foo' :
            expect(tinfra::json_token::COLON);
            next_or_fail("expected value after ':'");

            // try to parse matching field
            push_expected_name(key);
            tinfra::mo_mutate(v, *this);
            pop_expected_name();

            // now decide: next or end
            if( current.type == tinfra::json_token::COMMA ) {
                // , -> continue
                next();
                continue;
            }
            if( current.type == tinfra::json_token::OBJECT_END ) {
                // } -> break
                next();
                break;
            }
            fail("expected ',' or '}' after value when parsing dictionary");
        }
        tinfra::mo_mutate(v, *this);
    }

    template <typename S, typename T>
    void sequence(S sym, T& array)
    {
        if( !match_name(sym) || finished ) {
            return;
        }
        expect(tinfra::json_token::ARRAY_BEGIN);
        next_or_fail("expected ] or value when parsing array but EOF found");

        if( current.type == tinfra::json_token::ARRAY_END ) {
            // short circuit for []
            next();
            return;
        }
        while( true ) {
            {
                // ANYTHING
                typename T::value_type tmp;
                push_expected_name("!foo");
                tinfra::mo_mutate("!foo", tmp, *this);
                pop_expected_name();

                using std::swap;
                array.push_back(tmp);
            }

            // now decide: next or end
            if( current.type == tinfra::json_token::COMMA ) {
                // , -> continue
                next_or_fail("expected value after ','");
                continue;
            }
            if( current.type == tinfra::json_token::ARRAY_END ) {
                // ] -> break
                next();
                break;
            }
            fail("expected ',' or ']' after value when parsing array");
        }
    }
    template <typename S>
    void leaf(S sym, std::string& v)
        // std::string leaf
        // store any value as string, even integers and doubles
    {
        if( !match_name(sym) || finished ) {
            return;
        }

        switch( current.type ) {
        case tinfra::json_token::STRING:
        case tinfra::json_token::INTEGER:
        case tinfra::json_token::DOUBLE:
            v = current.value.str();
            next();
            break;
        default:
            fail(tsprintf("expected string for field '%s' but found (found %s)", get_full_field_name(sym), current.type));
        }
    }

    template <typename S, typename I>
    void integer_leaf(S sym, I& v)
        // integer type leaf leaf
        // store any value as string, even integers and doubles
    {
        if( !match_name(sym) || finished ) {
            return;
        }

        if( current.type == tinfra::json_token::INTEGER ) {
            v = tinfra::from_string<I>(current.value.str());
            next();
        } else {
            fail(tsprintf("expected integer for field '%s' but found (found %s)", get_full_field_name(sym), current.type));
        }
    }
    template <typename S>
    void leaf(S sym, int& v) {
        this->integer_leaf<int>(sym, v);
    }
    template <typename S>
    void leaf(S sym, long& v) {
        this->integer_leaf<long>(sym, v);
    }
    template <typename S>
    void leaf(S sym, long long& v) {
        this->integer_leaf<long long>(sym, v);
    }
private:
    void push_expected_name(std::string const& str);
    void pop_expected_name();
    bool match_name(tinfra::tstring const& name);

    bool next();
    void next_or_fail(const char* message);
    void fail(std::string const& message);
    void expect(tinfra::json_token::token_type tt);
    std::string get_full_field_name(tinfra::tstring sym);
};

// namespace tinfra

#endif // json_mo_parser_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
