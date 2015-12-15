PUBLIC	myshit
; Function compile flags: /Odtp
_TEXT	SEGMENT
a$ = 8
b$ = 16
myshit	PROC
xor rax, rax
add rax, rcx
add rax, rdx
ret
myshit	ENDP
_TEXT	ENDS
END
