#include "json_mo_parser.h"

#include "tinfra/fmt.h"

// namespace tinfra {

tinfra::module_tracer json_mo_parser_tracer(tinfra::tinfra_tracer, "json_mo_parser");

//
// json_mo_parser
//

using tinfra::tsprintf;

json_mo_parser::json_mo_parser(tinfra::json_lexer& lexer):
    lexer(lexer),
    finished(false)
{
    next();
}


void json_mo_parser::push_expected_name(std::string const& str) {
    this->name_stack.push_back(str);
}
void json_mo_parser::pop_expected_name() {
    this->name_stack.pop_back();
}
std::string json_mo_parser::get_full_field_name(tinfra::tstring sym)
{
    std::ostringstream builder;
    int i = 0;
    for( std::string const& item: this->name_stack) {
        if( i != 0 )
            builder << ".";
        builder << item;
    }
    if( i != 0 )
        builder << ".";
    builder << sym;
    return builder.str();
}
bool json_mo_parser::match_name(tinfra::tstring const& name) {
    if( name_stack.size() == 0 ) {
        // if name_stack is empty, then we're processing first
        // object. i.e root, so no name matching is needed
        return true;
    }
    if( name_stack[name_stack.size()-1] == name ) {
        return true;
    }
    return false;
}
bool json_mo_parser::next() {
    if( finished )
        return false;
    this->finished = !this->lexer.fetch_next(this->current);
    TINFRA_TRACE(json_mo_parser_tracer, "readed token " << this->current.type << " finished=" << finished);
    return !this->finished;
}

void json_mo_parser::next_or_fail(const char* message) 
{
    if( !next() ) {
        fail(message);
    }
}

void json_mo_parser::fail(std::string const& message)
{
    TINFRA_TRACE(json_mo_parser_tracer, "failure: " << message);
    throw std::runtime_error(tsprintf("json_parser: %s", message));
}

void json_mo_parser::expect(tinfra::json_token::token_type tt) 
{
    if( finished ) {
        fail(tsprintf("expecting %s, but end of input reached",
                                          tt));
    }
    if( this->current.type != tt ) {
        fail(tsprintf("expecting %s, but found %s",
             tt, this->current.type));
    }
}

// } // end namespace tinfra

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
