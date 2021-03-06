#pragma once

#include <memory>

#define NAME "Shooter"
#define MIN( a, b ) ((a) < (b) ? (a) : (b))
#define MAX( a, b ) ((a) > (b) ? (a) : (b))

extern uint16_t width, height;

template <typename T>
T throwonerror( T x, const char* error ){
    if( !x )
        throw std::runtime_error(error);
    return x;
}

template <typename T>
T throwonerror( T x, const char* error, T success ){
    if( x != success )
        throw std::runtime_error(error);
    return x;
}
