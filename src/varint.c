#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

__uint128_t encode(uint64_t);
uint64_t decode(__uint128_t);

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
    for (size_t i = 0; i < size; ++i) {
        printf("%zu: %hx ", i, buffer[i]);
    }
    printf("\n");

    // print decimal interpretation of each byte
    for (size_t i = 0; i < size; ++i) {
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
    for (int i = (int) size - 1; i >= 0; --i) {
        n += (uint64_t) buffer[i] << (pos++ * 8);
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

    __uint128_t encoded = encode(n);
    uint64_t decoded = decode(encoded);

    // assert(encoded == 0x01);    // 1
    // assert(encoded == 0x9601);  // 150
    // assert(encoded == ((__uint128_t) 1000000000000*1208925819614) 
    //     + 629174705921);  // 18446744073709551615 (maxint)

    // cannot use 0xFFFFFFFFFFFFFFFFFF01 literal
    // https://stackoverflow.com/a/31461428 and
    // https://discourse.llvm.org/t/is-there-any-status-regarding-128-bit-integer-support-in-clang/59889

    // solution: build up the desired value from smaller components
    // 0xFFFFFFFFFFFFFFFFFF01 == 1_208_925_819_614_629_174_705_921
    // __uint128_t n = (__uint128_t) (1_000_000_000_000*HI)+LOW
    // n = ((__uint128_t) 1000000000000*1208925819614) + 629174705921)

    assert(n == decoded);

    printf("encode: %lu\n", n);
    printf("decoded: %lu\n", decoded);

    fclose(fp);

    return EXIT_SUCCESS;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-invalid-specifier"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
__uint128_t encode(uint64_t src)
{
    // this encoding can use anywhere from 1 to 10 bytes
    unsigned char byte_seq[10];
    size_t count = 0;

    while (src > 0) {
        // take the lower 7 bits
        uint8_t byte = src & 0x7F;

        // move to the next 7 bits
        src >>= 7;

        // do we have more to encode?
        if (src > 0) {
            byte |= 1 << 7; // set continuation bit as msb
        }

        byte_seq[count] = byte;
        ++count;
    }

    __uint128_t n = 0;
    size_t pos = 0;

    // concatenate the bytes
    for (int i = (int) count - 1; i >= 0; --i) {
        n += (__uint128_t) byte_seq[i] << (pos++ * 8);
    }

    return n;
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-invalid-specifier"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
uint64_t decode(__uint128_t src)
{
    unsigned char byte_seq[10];
    size_t count = 0;

    while (src > 0) {
        uint8_t byte = src & 0x7F;

        src >>= 8;

        byte_seq[count] = byte;
        ++count;
    }

    uint64_t n = 0;
    size_t pos = 0;

    // concatenate the bytes
    for (int i = (int) count - 1; i >= 0; --i) {
        n += (uint64_t) byte_seq[i] << (pos++ * 7);
    }

    return n;
}
#pragma GCC diagnostic pop
