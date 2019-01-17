#pragma once

#include <memory>

template <typename T>
T throwonerror(T x, const char* error){
    if(!x)
        throw std::runtime_error(error);
    return x;
}


