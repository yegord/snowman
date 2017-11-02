/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "IdaPlugin.h"

#include <cassert>
#include <memory> /* std::unique_ptr */
#include <vector>

#include <nc/common/Foreach.h>

#include "IdaWorkaroundStart.h"
#include <ida.hpp>
#include <bytes.hpp>
#include <auto.hpp>
#include <ua.hpp>
#include <idp.hpp>     /* For IDP_INTERFACE_VERSION. */
#include <loader.hpp>  /* For PLUGIN_KEEP. */
#include "IdaWorkaroundEnd.h"

namespace nc { namespace ida { namespace detail {

namespace {
    struct IdaPluginStorage {
        std::vector<IdaPluginRegistrator::PluginFactory> pluginFactories;
        std::vector<std::unique_ptr<IdaPlugin>> plugins;
    };

    IdaPluginStorage *storage = NULL;
} // namespace `anonymous-namespace`


bool IdaPluginRegistrator::registerPlugin(const PluginFactory &pluginFactory) {
    assert(!pluginFactory.empty());

    if(storage == NULL)
        storage = new IdaPluginStorage(); /* No locking since it will be called at application startup time. */

    storage->pluginFactories.push_back(pluginFactory);

    return true;
}


namespace {
// -------------------------------------------------------------------------- //
// Plugin interface
// -------------------------------------------------------------------------- //
    char name[] = "Snowman";
    char help[] = "";
    char hotkey[] = "";
    char comment[] = "";

    int idaapi init() {
        /* Documentation says:
         * 
         * Do checks here to ensure your plug-in is being used within
         * an environment it was written for. Return PLUGIN_SKIP if the
         * checks fail, otherwise return PLUGIN_KEEP. */

        foreach (const IdaPluginRegistrator::PluginFactory &pluginFactory, storage->pluginFactories)
            storage->plugins.push_back(std::unique_ptr<IdaPlugin>(pluginFactory()));

        return PLUGIN_KEEP;
    }

    bool idaapi run(size_t /*arg*/) {
        /* Documentation says:
         * 
         * The plugin can be passed an integer argument from the plugins.cfg
         * file. This can be useful when you want the one plug-in to do
         * something different depending on the hot-key pressed or menu
         * item selected. */

        foreach (auto &plugin, storage->plugins) {
            (*plugin)();
        }
		return true;
    }

    void idaapi terminate() {
        /* Documentation says:
         * 
         * Stuff to do when exiting, generally you'd put any sort
         * of clean-up jobs here. */

        /* Perform destruction in the reverse order. */
        while (!storage->plugins.empty()) {
            storage->plugins.pop_back();
        }
    }

} // namespace `anonymous-namespace`
}}} // namespace nc::ida::detail

#ifdef Q_OS_WIN
__declspec(dllexport)
#endif
plugin_t PLUGIN = {
    IDP_INTERFACE_VERSION,      /**< IDA version plug-in is written for. */
    0,                          /**< Flags. */
    nc::ida::detail::init,      /**< Initialization function. */
    nc::ida::detail::terminate, /**< Clean-up function. */
    nc::ida::detail::run,       /**< Main plug-in body. */
    nc::ida::detail::comment,   /**< Comment. */
    nc::ida::detail::help,      /**< Help multi-line string. */
    nc::ida::detail::name,      /**< Plug-in name shown in Edit->Plugins menu. */
    nc::ida::detail::hotkey     /**< Hot key to run the plug-in */
};

/* vim:set et sts=4 sw=4: */
