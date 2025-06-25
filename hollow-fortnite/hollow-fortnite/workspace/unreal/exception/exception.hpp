#pragma once
#include <impl/includes.hpp>

namespace m_class {
    class m_exception_query {
    private:

    public:
        [[ nodiscard ]]
        static long __stdcall exception_filter(
            PEXCEPTION_POINTERS p_exception_pointers
        );
    };
}