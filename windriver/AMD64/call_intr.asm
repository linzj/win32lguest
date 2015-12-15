PUBLIC	call_intr
_TEXT	SEGMENT
call_intr	PROC
    call $LN1@call_intr
$LN1@call_intr:
    pop rax
    add rax, $LN2@call_intr - $LN1@call_intr
$LN2@call_intr:
    sti
	ret
call_intr ENDP
_TEXT	ENDS
END
