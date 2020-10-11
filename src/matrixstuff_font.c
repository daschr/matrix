#include "matrixstuff_font.h"
#include <stdint.h>
// bit patterns for the CP437 font

const uint64_t matrix_font[] = {
    0x0000000000000000,
    0x180018183c3c1800,
	0x0000000000363636,
    0x6c6cfe6cfe6c6c00,
    0x103c403804781000,
    0x60660c1830660600,
    0xfc66a6143c663c00,
    0x0000000000030303,
    0x6030181818306000,
    0x060c1818180c0600,
    0x006c38fe386c0000,
    0x0010107c10100000,
    0x060c0c0c00000000,
    0x0000003c00000000,
    0x0606000000000000,
    0x00060c1830600000, 
    0x3c66666e76663c00, //0
    0x7e1818181c181800,
    0x7e060c3060663c00,
    0x3c66603860663c00,
    0x30307e3234383000,
    0x3c6660603e067e00,
    0x3c66663e06663c00,
    0x1818183030667e00,
    0x3c66663c66663c00,
    0x3c66607c66663c00, // 9
    0x0018180018180000,
    0x0c18180018180000,
    0x6030180c18306000,
    0x00003c003c000000,
    0x060c1830180c0600,
    0x1800183860663c00, 
    0x003c421a3a221c00, //@
    0x6666667e66663c00, //A
    0x3e66663e66663e00,
    0x3c66060606663c00,
    0x3e66666666663e00,
    0x7e06063e06067e00,
    0x0606063e06067e00,
    0x3c66760606663c00,
    0x6666667e66666600,
    0x3c18181818183c00,
    0x1c36363030307800,
    0x66361e0e1e366600,
    0x7e06060606060600,
    0xc6c6c6d6feeec600,
    0xc6c6e6f6decec600,
    0x3c66666666663c00,
    0x06063e6666663e00,
    0x603c766666663c00,
    0x66361e3e66663e00,
    0x3c66603c06663c00,
    0x18181818185a7e00,
    0x7c66666666666600,
    0x183c666666666600,
    0xc6eefed6c6c6c600,
    0xc6c66c386cc6c600,
    0x1818183c66666600,
    0x7e060c1830607e00, //Z
    0x7818181818187800,
    0x006030180c060000,
    0x1e18181818181e00,
    0x0000008244281000,
    0xffff000000000000,
    0x0000000060303000,
    0x7c667c603c000000, //a
    0x3e66663e06060600,
    0x3c6606663c000000,
    0x7c66667c60606000,
    0x3c067e663c000000,
    0x0c0c3e0c0c6c3800,
    0x3c607c66667c0000,
    0x6666663e06060600,
    0x3c18181800180000,
    0x1c36363030003000,
    0x66361e3666060600,
    0x1818181818181800,
    0xd6d6feeec6000000,
    0x6666667e3e000000,
    0x3c6666663c000000,
    0x06063e66663e0000,
    0xf0b03c36363c0000,
    0x060666663e000000,
    0x3e403c027c000000,
    0x1818187e18180000,
    0x7c66666666000000,
    0x183c666600000000,
    0x7cd6d6d6c6000000,
    0x663c183c66000000,
    0x3c607c6666000000,
    0x3c0c18303c000000,
	0x3c6666663c006666, //ö
	0x7c66666666006666,	//ü
	0x7c667c603c006666  //ä
};
