global LoadIDT

global isr33
extern HandleKeyboardInterrupt

global isr32
extern PITInterruptHandler
extern Schedule
extern log_cpu_state

;global isr13
;extern gp_fault

LoadIDT:
	mov eax, [esp + 4]
	lidt[eax]
	ret

;isr13:
	;cli
	;pushad
	;call gp_fault
	;popad
	;add esp, 4
	;iret
	
isr32:
	cli
	pushad
	call PITInterruptHandler
	push esp
	call log_cpu_state
	call Schedule
	add esp, 4
	mov esp, eax
	popad
	iret

isr33:
	cli
	pushad
	call HandleKeyboardInterrupt
	popad
	iret
