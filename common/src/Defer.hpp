#pragma once

#include <type_traits>
#include <functional>
#include <utility>

namespace common{


template <typename C>
class Defer{
public:
    Defer(C callable_object)
        :   _callable_object(callable_object)
    {
        static_assert(std::is_invocable_v<C>, "C must be callable");
    }
    ~Defer(){
        _callable_object();
    }
private:
    C _callable_object;
};



}
