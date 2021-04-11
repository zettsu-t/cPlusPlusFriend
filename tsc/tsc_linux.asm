global GetCpuVersion ; void GetCpuVersion(CpuVersion* pResult)
global GetTscRatio   ; void GetTscRatio(uint32_t* pResult)
global GetTsc        ; uint64_t GetTsc(void)

GetCpuVersion:
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    mov eax, 0x1
    cpuid
    pop rdi
    mov dword [rdi], eax
    pop rdx
    pop rcx
    pop rbx
    pop rax
    ret

GetTscRatio:
    push rax
    push rbx
    push rcx
    push rdx
    mov eax, 0x15
    mov ebx, 1
    mov ecx, 1
    push rdi
    cpuid
    pop rdi
    mov dword [rdi], eax
    mov dword [rdi + 4], ebx
    pop rdx
    pop rcx
    pop rbx
    pop rax
    ret

GetTsc:
    push rdx
    rdtsc
    shl rdx, 32
    or  rax, rdx
    pop rdx
    ret
