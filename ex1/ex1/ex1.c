#include "ex1.h"
/*
 * Created by omerz on 20/11/17.
 * Omer Zucker
 * ID: 200876548
 */


int is_little_endian() {
    // we will check for the value 1
    int num = 1;
    // pointer- array of chars (each bit)
    char *index;

    // pointer at first byte of num - LSB or MSB
    index = (char*)&num;
    // if pointer on LSB so this is little endian - return 1
    // else, pointer is on MSB so this is big endian - return 0
    return (*index);

}

/*
 * merge bytes of x with bytes of y
 */
unsigned long merge_bytes(unsigned long x, unsigned long int y) {
    int i;
    unsigned long result;
    char *indexOfResult = (char *) &result;
    // points at first byte of x
    char *indexOfX = (char *) &x;
    // point at first byte of y
    char *indexOfY = (char *) &y;

    // for little endian
    if (is_little_endian()) {
        // MSB of y is 19
        indexOfResult[0] = indexOfY[0];
        // run through X digits
        for (i = 1; i < sizeof(long); i++) {
            indexOfResult[i] = indexOfX[i];
        }
        return result;
    }
    // for big endian
    else {
        // LSB of y is 19
        indexOfResult[sizeof(long) - 1] = indexOfY[sizeof(long) - 1];
        // run through X digits
        for (i = 0; i < sizeof(long) - 1; i++) {
            indexOfResult[i] = indexOfX[i];
        }
        return result;
    }
}

    /*
     * put a specific byte (value of b) in given index i in x
     */
    unsigned long put_byte(unsigned long x, unsigned char b, int i) {
        unsigned long result = x;
        char *indexOfResult = (char *) &result;
        // little endian
        if (is_little_endian()) {
            // copy value from x to result
            indexOfResult[i] = b;
            return result;
        }
        // big endian
        if (!is_little_endian()) {
            // copy values from x to result
            indexOfResult[sizeof(long)-(i+1)] = b;
            return result;
        }
    }

