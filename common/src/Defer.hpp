#pragma once

#include <type_traits>
#include <functional>
#include <utility>

template <typename C>
class Defer{
public:
    Defer(C callable_object)
        :   _callable_object(callable_object)
    {

    }
    ~Defer(){
        _callable_object();
    }
private:
    C _callable_object;
};
