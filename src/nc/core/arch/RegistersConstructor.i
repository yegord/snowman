/* No include guards. */

/**
 * \file
 *
 * This file is to be used inside a constructor of registers container. 
 * It generates the necessary boilerplate to fill the container.
 * For example usage, see <tt>X86Registers.cpp</tt>.
 */

/* Including this one in the middle of a function is OK. */
#include <boost/preprocessor/stringize.hpp>

#ifndef REGISTER_TABLE
#    error Define REGISTER_TABLE before including this file
#endif

#ifdef REG
#    error REG is expected not to be defined
#endif

{
    struct RegisterDescription {
        int number;
        const char *name;
        nc::core::ir::Domain domain;
        nc::BitSize offset;
        nc::BitSize size;
        const char *comment;
    };

    static const RegisterDescription registerDescriptions[] = {
#define REG(lowercase, uppercase, domain, offset, size, comment)                \
        { uppercase, BOOST_PP_STRINGIZE(lowercase), domain, offset, size, comment },
#include REGISTER_TABLE
#undef REG
    };

    for(unsigned i = 0; i < sizeof(registerDescriptions) / sizeof(RegisterDescription); i++) {
        registerRegister(
            new core::arch::Register(
                registerDescriptions[i].number,
                registerDescriptions[i].name,
                nc::core::ir::MemoryLocation(
                    nc::core::ir::MemoryDomain::FIRST_REGISTER + registerDescriptions[i].domain,
                    registerDescriptions[i].offset,
                    registerDescriptions[i].size
                )
            )
        );
    }
}

#undef REGISTER_TABLE
