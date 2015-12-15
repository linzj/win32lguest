
PUBLIC	flush_tlb
_TEXT	SEGMENT
flush_tlb	PROC
    mov rax, cr3
    mov cr3, rax
    xor rax, rax
	ret
flush_tlb ENDP
_TEXT	ENDS
END

