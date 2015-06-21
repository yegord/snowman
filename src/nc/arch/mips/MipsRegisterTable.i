/**
 * \file
 *
 * Register table for MIPS architecture.
 *
 * This header is intended for multiple inclusion.
 */


/**
 * \def REG(lowercase, uppercase, domain, offset, size, comment)
 *
 * 
 * \param lowercase                    Name of the register as a lowercase 
 *                                     C++ identifier.
 * \param uppercase                    Name of the register as an UPPERCASE
 *                                     C++ identifier.
 * \param domain                       Number of register's domain.
 * \param offset                       Bit offset of the register inside
 *                                     the domain.
 * \param size                         Bit size of the register.
 * \param comment                      Register description.
 */
REG(zero,	ZERO,	0,	0,	32, "Always contains zero")
REG(at,     AT, 	0,	1,	32, "Assembler temporary register")
REG(v0,     V0, 	0,	2,	32, "Return value 0")
REG(v1,     V1, 	0,	3,	32, "Return value 1")
REG(a0,     A0, 	0,	4,	32, "Calling value 0")
REG(a1,     A1, 	0,	5,	32, "Calling value 1")
REG(a2,     A2, 	0,	6,	32, "Calling value 2")
REG(a3,     A3, 	0,	7,	32, "Calling value 3")
REG(t0,     T0, 	0,	8,	32, "Temporary register")
REG(t1,     T1, 	0,	9,	32, "Temporary register")
REG(t2,     T2, 	0,	10,	32, "Temporary register")
REG(t3,     T3, 	0,	11,	32, "Temporary register")
REG(t4,     T4, 	0,  12,	32, "Temporary register")
REG(t5,     T5, 	0,  13,	32, "Temporary register")
REG(t6,     T6, 	0,  14,	32, "Temporary register")
REG(t7,     T7, 	0,  15,	32, "Temporary register")
REG(s0,     S0, 	0,  16,	32, "Callee saved register")
REG(s1,     S1, 	0,  17,	32, "Callee saved register")
REG(s2,     S2, 	0,  18,	32, "Callee saved register")
REG(s3,     S3, 	0,  19,	32, "Callee saved register")
REG(s4,     S4, 	0,  20,	32, "Callee saved register")
REG(s5,     S5, 	0,  21,	32, "Callee saved register")
REG(s6,     S6, 	0,  22,	32, "Callee saved register")
REG(s7,     S7, 	0,  23,	32, "Callee saved register")
REG(t8,     T8, 	0,  24,	32, "Temporary register")
REG(t9,     T9, 	0,  25,	32, "Temporary register")
REG(k0,     K0, 	0,  26,	32, "Reserved for kernel trap / IRQ handling")
REG(k1,     K1, 	0,  27,	32, "Reserved for kernel trap / IRQ handling")
REG(gp,     GP, 	0,  28,	32, "Global pointer")
REG(sp,     SP, 	0,  29,	32, "Stack pointer")
REG(fp,     FP, 	0,  30,	32, "Frame pointer")
REG(s8,     S8, 	1,  30,	32, "Calle saved register / frame pointer")
REG(ra,     RA, 	0,  31,	32, "Return Address")

/* vim:set et sts=4 sw=4: */
