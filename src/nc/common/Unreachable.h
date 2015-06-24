/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <cassert>

/**
 * Macro for marking unreachable code.
 */
#define unreachable() do { assert(!"Unreachable code executed."); } while (true)

/* vim:set et sts=4 sw=4: */
