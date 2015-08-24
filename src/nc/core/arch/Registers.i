/* No include guards. */

/**
 * \file
 *
 * This file is to be used to generate register enumeration and accessors 
 * from register table. For example usage, see <tt>X86Registers.h</tt>.
 */

#ifndef REGISTER_TABLE
#    error Define REGISTER_TABLE before including this file
#endif

#ifdef REG
#    error REG is expected not to be defined
#endif

    /* Register numbers. */
    enum {
#define REG(lowercase, uppercase, domain, offset, size, comment)\
        uppercase,
#include REGISTER_TABLE
#undef REG
        INVALID = nc::core::arch::Registers::INVALID
    };

    /* Register accessors. */
#define REG(lowercase, uppercase, domain, offset, size, comment)\
    static const nc::core::arch::Register *lowercase() {        \
        return getRegister(uppercase);                          \
    }
#include REGISTER_TABLE
#undef REG

#undef REGISTER_TABLE

/* vim:set et sts=4 sw=4: */
