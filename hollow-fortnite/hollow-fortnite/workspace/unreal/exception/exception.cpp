#include <workspace/unreal/exception/exception.hpp>

[[ nodiscard ]]
long __stdcall m_class::m_exception_query::exception_filter(
    PEXCEPTION_POINTERS p_exception_pointers
) {
    const auto* p_context = p_exception_pointers->ContextRecord;
    char message[1024];

    sprintf(message,
        encrypt("the service has been stopped due to a fatal error.\n\n"
            "if this is the first time you've seen this error message, restart the service.\n"
            "if this message appears again, follow these steps:\n\n"
            "rollback any newly downloaded windows updates.\n\n"
            "if nothing helps, please contact sqqrky.\n\n"
            "technical information:\n"
            "build type: 0 (built on %s %s)\n"
            "error code: 0x%08X\n"
            "address: %p\n"
            "attempt to read data at address: 0x0\n\n"
            "esp = 0x%016llX\n"
            "edi = 0x%016llX\n"
            "esi = 0x%016llX\n"
            "ebx = 0x%016llX\n"
            "edx = 0x%016llX\n"
            "ecx = 0x%016llX\n"
            "eax = 0x%016llX\n"
            "ebp = 0x%016llX"),
        __DATE__, __TIME__,
        p_exception_pointers->ExceptionRecord->ExceptionCode,
        p_exception_pointers->ExceptionRecord->ExceptionAddress,
        p_context->Rsp,
        p_context->Rdi,
        p_context->Rsi,
        p_context->Rbx,
        p_context->Rdx,
        p_context->Rcx,
        p_context->Rax,
        p_context->Rbp
    );

    printf(encrypt("\n"));
    printf(encrypt("stopped due to a fatal error, uninstalling service"));
    kernel->remove_exception();

    MessageBoxA(0, message, "the service has crashed!", MB_ICONERROR | MB_OK);
    exit(0);

    return EXCEPTION_CONTINUE_EXECUTION;
}