/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <boost/preprocessor/cat.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/functional/factory.hpp>

#include <nc/common/Unused.h>

namespace nc { namespace ida {

/**
 * Base class for ida plugins.
 */
class IdaPlugin: public boost::noncopyable {
public:
    /**
     * Main plugin function. Will be called upon plugin invocation from IDA.
     */
    virtual void operator()() {}

    virtual ~IdaPlugin() {}
};


namespace detail {

class IdaPluginRegistrator {
public:
    typedef boost::function<IdaPlugin *()> PluginFactory;

    static bool registerPlugin(const PluginFactory &pluginFactory);
};

} // namespace detail


/**
 * This macro registers the given type as an ida plugin. 
 *
 * It will be constructed upon ida initialization and destructed upon ida
 * deinitialization.
 *
 * \param plugin_class                 Ida plugin class to register.
 */
#define NC_IDA_REGISTER_PLUGIN(plugin_class)                                    \
static_assert(                                                                  \
    boost::is_base_of< ::nc::ida::IdaPlugin, plugin_class>::value,              \
    "Plugins registered with NC_IDA_REGISTER_PLUGIN macro must implement IdaPlugin interface." \
);                                                                              \
static bool BOOST_PP_CAT(registered_, plugin_class) =                           \
    (NC_UNUSED(BOOST_PP_CAT(registered_, plugin_class)),                        \
     ::nc::ida::detail::IdaPluginRegistrator::registerPlugin(boost::factory<plugin_class *>()));\

}} // namespace nc::ida

/* vim:set et sts=4 sw=4: */
