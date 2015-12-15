PUBLIC	co_i386_has_cpuid
_TEXT	SEGMENT
co_i386_has_cpuid	PROC
;;Try change the CPUID FLAGS bit (put the old FLAGS in RCX)
    pushf  
    pop    rax
    mov    rcx,rax

    xor    rax, 200000h
    push   rax
    popf   
;;Read the new FLAGS (and restore the FLAGS from RCX)
    pushf  
    pop    rax
    xor    rax, rcx
    push   rcx
    popf

    ;;Was the bit changed?
    xor rdx, rdx ;; init the auto variable

    and    rax, 200000h
    je     $EXIT@co_i386_has_cpuid
    mov    rdx, 1
$EXIT@co_i386_has_cpuid:
    mov rax, rdx
    ret    
co_i386_has_cpuid ENDP
_TEXT	ENDS
END

PUBLIC	co_i386_get_cpuid
_TEXT	SEGMENT
co_i386_get_cpuid	PROC
    push rbx
    push rsi
    push rdx
    mov rax, rcx
    cpuid
    mov rsi, [rsp]
    mov DWORD PTR [rsi], eax
    mov DWORD PTR [rsi + 4], ebx
    mov DWORD PTR [rsi + 8], ecx
    mov DWORD PTR [rsi + 12], edx
    pop rdx
    pop rsi
    pop rbx
    ret
co_i386_get_cpuid ENDP
_TEXT	ENDS
END
