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

/**
 * \file
 *
 * IDA headers define some functions that conflict with definitions from Qt.
 * So we trick compiler by &#35;defining them to functions with different names.
 *
 * Note that this header must be include BEFORE any of the IDA headers.
 */

/*
 * Include standard headers using pid_t.
 * We don't want our preprocessor definitions to influence them.
 */
#ifdef _MSC_VER
#include <process.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
#include <time.h>
#endif

#ifdef __MAC__
#define USE_DANGEROUS_FUNCTIONS
#include <fcntl.h>
#endif

#define qstrdup ida_qstrdup
#define qstrncpy ida_qstrncpy
#define qvsnprintf ida_qvsnprintf
#define qsnprintf ida_qsnprintf
#define qstrlen ida_qstrlen
#define qstrcmp ida_qstrcmp
#define ulong ida_ulong
#define pid_t ida_pid_t
