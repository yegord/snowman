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
REG(zero,	ZERO,	0,	0  * 32,	32, "Always contains zero")
REG(at,     AT, 	0,	1  * 32,	32, "Assembler temporary register")
REG(v0,     V0, 	0,	2  * 32,	32, "Return value 0")
REG(v1,     V1, 	0,	3  * 32,	32, "Return value 1")
REG(a0,     A0, 	0,	4  * 32,	32, "Calling value 0")
REG(a1,     A1, 	0,	5  * 32,	32, "Calling value 1")
REG(a2,     A2, 	0,	6  * 32,	32, "Calling value 2")
REG(a3,     A3, 	0,	7  * 32,	32, "Calling value 3")
REG(t0,     T0, 	0,	8  * 32,	32, "Temporary register")
REG(t1,     T1, 	0,	9  * 32,	32, "Temporary register")
REG(t2,     T2, 	0,	10 * 32,	32, "Temporary register")
REG(t3,     T3, 	0,	11 * 32,	32, "Temporary register")
REG(t4,     T4, 	0,  12 * 32,	32, "Temporary register")
REG(t5,     T5, 	0,  13 * 32,	32, "Temporary register")
REG(t6,     T6, 	0,  14 * 32,	32, "Temporary register")
REG(t7,     T7, 	0,  15 * 32,	32, "Temporary register")
REG(s0,     S0, 	0,  16 * 32,	32, "Callee saved register")
REG(s1,     S1, 	0,  17 * 32,	32, "Callee saved register")
REG(s2,     S2, 	0,  18 * 32,	32, "Callee saved register")
REG(s3,     S3, 	0,  19 * 32,	32, "Callee saved register")
REG(s4,     S4, 	0,  20 * 32,	32, "Callee saved register")
REG(s5,     S5, 	0,  21 * 32,	32, "Callee saved register")
REG(s6,     S6, 	0,  22 * 32,	32, "Callee saved register")
REG(s7,     S7, 	0,  23 * 32,	32, "Callee saved register")
REG(t8,     T8, 	0,  24 * 32,	32, "Temporary register")
REG(t9,     T9, 	0,  25 * 32,	32, "Temporary register")
REG(k0,     K0, 	0,  26 * 32,	32, "Reserved for kernel trap / IRQ handling")
REG(k1,     K1, 	0,  27 * 32,	32, "Reserved for kernel trap / IRQ handling")
REG(gp,     GP, 	0,  28 * 32,	32, "Global pointer")
REG(sp,     SP, 	0,  29 * 32,	32, "Stack pointer")
REG(fp,     FP, 	0,  30 * 32,	32, "Frame pointer")
REG(s8,     S8, 	0,  30 * 32,	32, "Calle saved register / frame pointer")
REG(ra,     RA, 	0,  31 * 32,	32, "Return Address")

REG(f0,		F0,		1,	0  * 32,	32, "Float point register")
REG(f1,		F1,		1,	1  * 32,	32, "Float point register")
REG(f2,		F2,		1,	2  * 32,	32, "Float point register")
REG(f3,		F3,		1,	3  * 32,	32, "Float point register")
REG(f4,		F4,		1,	4  * 32,	32, "Float point register")
REG(f5,		F5,		1,	5  * 32,	32, "Float point register")
REG(f6,		F6,		1,	6  * 32,	32, "Float point register")
REG(f7,		F7,		1,	7  * 32,	32, "Float point register")
REG(f8,		F8,		1,	8  * 32,	32, "Float point register")
REG(f9,		F9,		1,	9  * 32,	32, "Float point register")
REG(f10,	F10,	1,	10 * 32,	32, "Float point register")
REG(f11,	F11,	1,	11 * 32,	32, "Float point register")
REG(f12,	F12,	1,	12 * 32,	32, "Float point register")
REG(fa0,	FA0,	1,	12 * 32,	32, "Float point register")
REG(f13,	F13,	1,	13 * 32,	32, "Float point register")
REG(f14,	F14,	1,	14 * 32,	32, "Float point register")
REG(fa1,	FA1,	1,	14 * 32,	32, "Float point register")
REG(f15,	F15,	1,	15 * 32,	32, "Float point register")
REG(f16,	F16,	1,	16 * 32,	32, "Float point register")
REG(f17,	F17,	1,	17 * 32,	32, "Float point register")
REG(f18,	F18,	1,	18 * 32,	32, "Float point register")
REG(f19,	F19,	1,	19 * 32,	32, "Float point register")
REG(f20,	F20,	1,	20 * 32,	32, "Float point register")
REG(f21,	F21,	1,	21 * 32,	32, "Float point register")
REG(f22,	F22,	1,	22 * 32,	32, "Float point register")
REG(f23,	F23,	1,	23 * 32,	32, "Float point register")
REG(f24,	F24,	1,	24 * 32,	32, "Float point register")
REG(f25,	F25,	1,	25 * 32,	32, "Float point register")
REG(f26,	F26,	1,	26 * 32,	32, "Float point register")
REG(f27,	F27,	1,	27 * 32,	32, "Float point register")
REG(f28,	F28,	1,	28 * 32,	32, "Float point register")
REG(f29,	F29,	1,	29 * 32,	32, "Float point register")
REG(f30,	F30,	1,	30 * 32,	32, "Float point register")
REG(f31,	F31,	1,	31 * 32,	32, "Float point register")

REG(hilo,   HILO, 	2,  0  * 64,	64, "Pair of 32-bits == 64 bits")
REG(lo,     LO, 	2,  0  * 32,	32, "Low half result")
REG(hi,     HI, 	2,  1  * 32,	32, "High half result")

REG(cflag,  CFLAG,  3,  0  *  1,     1, "")
/* vim:set et sts=4 sw=4: */
