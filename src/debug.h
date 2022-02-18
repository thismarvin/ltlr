#pragma once

#include <stdio.h>

#define PRINTF_FORMAT_SPECIFIER(value) _Generic((value), \
    char: "%c", \
    signed char: "%hhd", \
    unsigned char: "%hhu", \
    short: "%hd", \
    unsigned short: "%hu", \
    int: "%d", \
    unsigned int: "%u", \
    long int: "%ld", \
    unsigned long int: "%lu", \
    long long int: "%lld", \
    unsigned long long int: "%llu", \
    float: "%f", \
    double: "%f", \
    long double: "%Lf", \
    char*: "%s", \
    void*: "%p")

#define PRINT(value) printf(PRINTF_FORMAT_SPECIFIER(value), value)
#define PRINTLN(value) printf(PRINTF_FORMAT_SPECIFIER(value), value); printf("\n")

char DebugChar(char value);
signed char DebugSignedChar(signed char value);
unsigned char DebugUnsignedChar(unsigned char value);
short DebugShort(short value);
unsigned short DebugUnsignedShort(unsigned short value);
int DebugInt(int value);
unsigned int DebugUnsignedInt(unsigned int value);
long int DebugLongInt(long int value);
unsigned long int DebugUnsignedLongInt(unsigned long int value);
long long int DebugLongLongInt(long long int value);
unsigned long long int DebugUnsignedLongLongInt(unsigned long long int value);
float DebugFloat(float value);
double DebugDouble(double value);
long double DebugLongDouble(long double value);
char* DebugCharStar(char* value);
void* DebugVoidStar(void* value);

#define DBG(value) _Generic((value), \
    char: DebugChar, \
    signed char: DebugSignedChar, \
    unsigned char: DebugUnsignedChar, \
    short: DebugShort, \
    unsigned short: DebugUnsignedShort, \
    int: DebugInt, \
    unsigned int: DebugUnsignedInt, \
    long int: DebugLongInt, \
    unsigned long int: DebugUnsignedLongInt, \
    long long int: DebugLongLongInt, \
    unsigned long long int: DebugUnsignedLongLongInt, \
    float: DebugFloat, \
    double: DebugDouble, \
    long double: DebugLongDouble, \
    char*: DebugCharStar, \
    void*: DebugVoidStar \
    )(value)
