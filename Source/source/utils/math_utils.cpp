/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Math-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#include <algorithm>
#include <cmath>
#include <math.h>

#include "math_utils.h"


/* ----------------------------------------------------------------------------
 * Limits the given number to the given range, inclusive.
 * number:
 *   Number to clamp.
 * minimum:
 *   Minimum value it can have, inclusive.
 * maximum:
 *   Maximum value it can have, inclusive.
 */
float clamp(const float number, const float minimum, const float maximum) {
    return std::min(maximum, std::max(minimum, number));
}


/* ----------------------------------------------------------------------------
 * Given an input, it returns a 32-bit unsigned integer hash of that input.
 * input:
 *   The input number.
 */
uint32_t hash_nr(const unsigned int input) {
    //Robert Jenkins' 32 bit integer hash function.
    //From https://gist.github.com/badboy/6267743
    //This algorithm is the simplest, lightest, fairest one I could find.
    uint32_t n = (input + 0x7ED55D16) + (input << 12);
    n = (n ^ 0xC761C23C) ^ (n >> 19);
    n = (n + 0x165667B1) + (n << 5);
    n = (n + 0xD3A2646C) ^ (n << 9);
    n = (n + 0xFD7046C5) + (n << 3);
    n = (n ^ 0xB55A4F09) ^ (n >> 16);
    return n;
}


/* ----------------------------------------------------------------------------
 * Given two inputs, it returns a 32-bit unsigned integer hash of those inputs.
 * input1:
 *   First input number.
 * input2:
 *   Second input number.
 */
uint32_t hash_nr2(const unsigned int input1, const unsigned int input2) {
    uint32_t n1 = hash_nr(input1);
    
    //Same algorithm as in hash_nr() with one argument,
    //but I changed the magic numbers to other random stuff.
    uint32_t n2 = (input2 + 0x5D795E0E) + (input2 << 12);
    n2 = (n2 ^ 0xC07C34BD) ^ (n2 >> 19);
    n2 = (n2 + 0x4969B10A) + (n2 << 5);
    n2 = (n2 + 0x583EB559) ^ (n2 << 9);
    n2 = (n2 + 0x72F56900) + (n2 << 3);
    n2 = (n2 ^ 0x8B121972) ^ (n2 >> 16);
    
    return n1 * n2;
}


/* ----------------------------------------------------------------------------
 * Returns the interpolation between two numbers, given a number in an interval.
 * input:
 *   The input number.
 * input_start:
 *   Start of the interval the input number falls on, inclusive.
 *   The closer to input_start, the closer the output is to output_start.
 * input_end:
 *   End of the interval the number falls on, inclusive.
 * output_start:
 *   Number on the starting tip of the interpolation.
 * output_end:
 *   Number on the ending tip of the interpolation.
 */
float interpolate_number(
    const float input, const float input_start, const float input_end,
    const float output_start, const float output_end
) {
    return
        output_start +
        ((input - input_start) / (float) (input_end - input_start)) *
        (output_end - output_start);
}


/* ----------------------------------------------------------------------------
 * Returns a random float between the provided range, inclusive.
 * minimum:
 *   Minimum value that can be generated, inclusive.
 * maximum:
 *   Maximum value that can be generated, inclusive.
 */
float randomf(float minimum, float maximum) {
    if(minimum == maximum) return minimum;
    if(minimum > maximum) std::swap(minimum, maximum);
    return (float) rand() / ((float) RAND_MAX / (maximum - minimum)) + minimum;
}


/* ----------------------------------------------------------------------------
 * Returns a random integer between the provided range, inclusive.
 * minimum:
 *   Minimum value that can be generated, inclusive.
 * maximum:
 *   Maximum value that can be generated, inclusive.
 */
int randomi(int minimum, int maximum) {
    if(minimum == maximum) return minimum;
    if(minimum > maximum) std::swap(minimum, maximum);
    return ((rand()) % (maximum - minimum + 1)) + minimum;
}


/* ----------------------------------------------------------------------------
 * Sums a number to another (even if negative), and then
 * wraps that number across a limit, applying a modulus operation.
 * nr:
 *   Base number.
 * sum:
 *   Number to add (or subtract).
 * wrap_limit:
 *   Wrap between [0 - wrap_limit[.
 */
int sum_and_wrap(const int nr, const int sum, const int wrap_limit) {
    int final_nr = nr + sum;
    while(final_nr < 0) {
        final_nr += wrap_limit;
    }
    return final_nr % wrap_limit;
}


/* ----------------------------------------------------------------------------
 * Wraps a floating point number between the specified interval.
 * nr:
 *   Base number.
 * minimum:
 *   Minimum of the interval.
 * maximum:
 *   Maximum of the interval.
 */
float wrap_float(const float nr, const float minimum, const float maximum) {
    const float diff = maximum - minimum;
    return minimum + fmod(diff + fmod(nr - minimum, diff), diff);
}
