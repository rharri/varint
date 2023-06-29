#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

void encode(uint64_t);

int main(int argc, char const *argv[])
{
    if (argc == 1) {
        fprintf(stderr, "please provide a file name\n");
        return EXIT_FAILURE;
    }

    // accept a file name from supplied args
    char const *fname = argv[1];

    // open a read-only binary stream
    FILE *fp = fopen(fname, "rb");

    if (fp == NULL) {
        fprintf(stderr, "cannot open file\n");
        return EXIT_FAILURE;
    }

    size_t size = 8;
    unsigned char buffer[size]; // 8 byte array

    /*
        buffer:
        [0x00] [0x00] [0x00] [0x00] [0x00] [0x00] [0x00] [0x00]

        char = 1 byte
        unsigned = 0 - 255
    */

    // read an 8 byte block from the binary stream
    fread(buffer, size, 1, fp);

    // print hex interpretation of each byte
    for (size_t i = 0; i<size; ++i) {
        printf("%zu: %hx ", i, buffer[i]);
    }
    printf("\n");

    // print decimal interpretation of each byte
    for (size_t i = 0; i<size; ++i) {
        int hi = buffer[i] & 0xF0; // bit mask 1111 XXXX
        int lo = buffer[i] & 0x0F; // bit mask XXXX 1111
        printf("%zu: %d + %d = %d\n", i, hi, lo, hi+lo);
    }

    /*
        buffer:
        [0x00] [0x00] [0x00] [0x00] [0x00] [0x00] [0x00] [0x96]
    */

    // interpret bytes as uint64_t by summing each byte according
    // to their position in the 8 byte sequence
    uint64_t n = 0;
    size_t pos = 0;

    // ensure size can fit within an int since size_t is unsinged
    assert(size <= INT_MAX); // loss of precision was intentional

    // bytes are stored as big-endian, so start from last index
    for (int i = (int)size-1; i>=0; --i) {
        n += (uint64_t)buffer[i] << (pos++ * 8);
    }

    // §6.5.7 Bitwise shift operators:
    // "The integer promotions are performed on each of the operands. 
    // The type of the result is that of the promoted left operand."
    // char c = 1;
    // printf("%d", c << 8); // expression (c << 8) is promoted to int (256)

    /*
        E.g. Convert 2 bytes (0xFF00) to int

        ARRAY ---> [0x00] [0xFF] (big-endian)

        0xFF = 0b1111_1111

        0b1111_1111 << (1 * 8)
        0b1111_1111_0000_0000

            0 += 65280

        0x00 = 0b0000_0000

        0b0000_0000 << (0 * 8)
        0b0000_0000_0000_0000

            65280 += 0

        0x00FF = 65280
    
    */

    encode(n);

    fclose(fp);

    return EXIT_SUCCESS;
}

void encode(uint64_t src)
{
    // print uint64_t interpretation of bytes
    printf("%lu\n", src);
}
