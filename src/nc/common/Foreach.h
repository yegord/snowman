/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

/* Undefine qt foreach. */
#ifdef foreach
#   undef foreach
#endif

#ifdef reverse_foreach
#   undef reverse_foreach
#endif

#ifdef NC_HAVE_RANGE_BASED_FOR
#   include <boost/range/adaptor/reversed.hpp>
#   define foreach(a, b) for(a : b)
#   define reverse_foreach(a, b) for(a : boost::adaptors::reverse(b))
#else
#   include <boost/foreach.hpp>
#   define foreach BOOST_FOREACH
#   define reverse_foreach BOOST_REVERSE_FOREACH
#endif

/* vim:set et sts=4 sw=4: */
