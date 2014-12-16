//
// Copyright (c) 2014, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef json_mo_writer_h_included
#define json_mo_writer_h_included

#include "tinfra/json.h" // for json_renderer, json_writer
#include "tinfra/mo.h"     // for mo_process
#include "tinfra/stream.h" // for output_stream

// namespace tinfra { // TBD, move it to tinfra
class json_mo_writer {
    tinfra::output_stream& out;
    tinfra::json_renderer renderer;
    tinfra::json_writer writer;

public:
    json_mo_writer(tinfra::output_stream& o):
        out(o),
        renderer(out),
        writer(renderer)
    {}

    // MO contract
    template <typename S, class T>
    void leaf(S sym, T const& t) 
    {
        if( writer.expected_value_kind() == tinfra::json_writer::UNNAMED ) {
            writer.value(t);
        } else {
            writer.named_value(sym, t);
        }
    }

    template <typename S, typename T>
    void record(S sym, T const& v)
    {
        if( writer.expected_value_kind() == tinfra::json_writer::UNNAMED ) {
            writer.begin_object();
        } else {
            writer.named_begin_object(sym);
        }
        tinfra::mo_process(v, *this);
        writer.end_object();
    }

    template <typename S, typename T>
    void sequence(S sym, T const& v)
    {
        if( writer.expected_value_kind() == tinfra::json_writer::UNNAMED ) {
            writer.begin_array();
        } else {
            writer.named_begin_array(sym);
        }

        typedef typename T::const_iterator  iterator;
        for( iterator i = v.begin(); i != v.end(); ++i ) {
            tinfra::process(S(0),*i, *this);
        }

        writer.end_array();
    }
}; 

//} // end namespace tinfra

#endif // json_mo_writer_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
