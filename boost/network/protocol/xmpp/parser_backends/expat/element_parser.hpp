//            Copyright (c) Glyn Matthews 2010.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)


#ifndef __BOOST_NETWORK_PROTOCOL_XMPP_PARSER_BACKENDS_EXPAT_ELEMENT_PARSER_INC__
# define __BOOST_NETWORK_PROTOCOL_XMPP_PARSER_BACKENDS_EXPAT_ELEMENT_PARSER_INC__


# include <boost/network/traits/string.hpp>
# include <boost/network/protocol/xmpp/element.hpp>
# include <expat.h>
# include <cstring>


namespace boost {
namespace network {
namespace xmpp {
template <
    class Tag
    >
class basic_expat_element_parser {
public:

    typedef typename string<Tag>::type string_type;

    typedef basic_element<Tag> element_type;

    basic_expat_element_parser() {
        parser_ = XML_ParserCreate(NULL);
        // handle the case where the parser is NULL
        element_ = 0;
        depth_ = 0;

        XML_SetUserData(parser_, this);
        XML_SetElementHandler(parser_, start_element, end_element);
        XML_SetCharacterDataHandler(parser_, cdata);
    }

    ~basic_expat_element_parser() {
        XML_ParserFree(parser_);
    }

    bool feed(const string_type &chunk) {
        return feed(chunk, 0);
    }
    
    bool feed(const string_type &chunk, element_type *element) {
        element_ = element;
        return XML_Parse(parser_, chunk.c_str(), chunk.size(), 0) != 0;
    }

private:

    static void set_name(element_type *element, const XML_Char *name) {
        const XML_Char *name_begin = name;
        const XML_Char *name_end = name_begin + std::strlen(name_begin);

        element->set_name(string_type(name_begin, name_end));
    }

    static void set_attributes(element_type *element, const XML_Char **attrs) {
        if (attrs) {
            for (int i = 0; attrs[i]; i += 2) {
                const XML_Char *key_begin = attrs[i];
                const XML_Char *key_end = key_begin + std::strlen(key_begin);

                const XML_Char *val_begin = attrs[i + 1];
                const XML_Char *val_end = val_begin + std::strlen(val_begin);

                element->set_attribute(string_type(key_begin, key_end),
                                       string_type(val_begin, val_end));
            }
        }
    }

    static void start_element(void *userdata,
                              const XML_Char *name,
                              const XML_Char **attrs) {
        basic_expat_element_parser<Tag> *parser
            = static_cast<basic_expat_element_parser<Tag> *>(userdata);

        if (parser->depth_ == 1) {
            set_name(parser->element_, name);
            set_attributes(parser->element_, attrs);
        }
        else if (parser->depth_ > 1) {
            element_type *child = new element_type;
            set_name(child, name);
            set_attributes(child, attrs);
            parser->element_->add_child(child);
        }
        
        ++parser->depth_;
    }

    static void end_element(void *userdata,
                            const XML_Char *name) {
        basic_expat_element_parser<Tag> *parser
            = static_cast<basic_expat_element_parser<Tag> *>(userdata);

        --parser->depth_;
    }

    static void cdata(void *userdata,
                      const XML_Char *s,
                      int len) {
        basic_expat_element_parser<Tag> *parser
            = static_cast<basic_expat_element_parser<Tag> *>(userdata);

        parser->element_->add_child(
            new element_type(typename element_type::text(), string_type(s, s + len)));
    }

    XML_Parser parser_;
    element_type *element_;
    int depth_;

};
} // namespace xmpp
} // namespace network
} // namespace boost


#endif // __BOOST_NETWORK_PROTOCOL_XMPP_PARSER_BACKENDS_EXPAT_ELEMENT_PARSER_INC__
