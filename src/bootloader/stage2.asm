[bits 16]
[org 0x8000]

KERNEL_BOOT_VBE_INFO equ 0x9500

SMAP equ 0x534D4150
MMAP_COUNT equ 0x500
MMAP_BUFFER equ 0x502

main:
	mov [stage2boot_drive], dl
	mov si, stage2_message
	call print
	call get_memory_map
	call readKernel
	call enableA20
	call InitVESAGraphics
	call init32mode
	jmp $

enableA20:
	in al, 0x92
	or al, 2
	out 0x92, al
	mov si, A20enabled
	call print
	ret

InitVESAGraphics:
	xor ax, ax
	mov es, ax
	mov ds, ax
	
	mov ax, 0x4F01
	mov cx, 0x4112
	mov di, VBE_mode_info
	int 0x10
	cmp ax, 0x004F
	jne .error

	mov eax, [VBE_mode_info + 40]
	mov [KERNEL_BOOT_VBE_INFO], eax
	mov ax, [VBE_mode_info + 18]
	mov [KERNEL_BOOT_VBE_INFO + 4], ax
	mov ax, [VBE_mode_info + 20]
	mov [KERNEL_BOOT_VBE_INFO + 6], ax
	mov al, [VBE_mode_info + 25]
	mov [KERNEL_BOOT_VBE_INFO + 8], al
	mov ax, [VBE_mode_info + 16]
	mov [KERNEL_BOOT_VBE_INFO + 9], ax
	
	mov ax, 0x4F02
	mov bx, 0x4112
	int 0x10
	cmp ax, 0x004F
	jne .error
	
	ret

.error:
	mov si, vesa_failed
	call print
	jmp $

get_memory_map:
	xor ax, ax
	mov es, ax
	mov edi, MMAP_BUFFER
	xor ebx, ebx
	xor bp, bp

.loop:
	mov eax, 0xE820
	mov ecx, 24
	mov edx, SMAP

	int 0x15
	jc .loop_done

	cmp eax, SMAP
	jne .error

	inc bp
	add edi, 24
	
	test ebx, ebx
	jz .loop_done

	jmp .loop

.loop_done:
	mov [MMAP_COUNT], bp
	mov si, e820_success
	call print
	ret
.error:
	mov si, e820_error
	call print
	jmp $	

readKernel:
	mov ah, 0x42
	mov dl, [stage2boot_drive]
	mov si, dap
	int 0x13
	jc .read_error

	mov si, read1_success	
	call print
	ret
.read_error:
	mov si, disk_error
	call print
	jmp $

init32mode:
	cli 
	lgdt[GDT]
	mov eax, cr0
	or eax, 0x1
	mov cr0, eax

	jmp CODESEG:.enable32bitmode

[bits 32]
.enable32bitmode:
	mov ax, DATASEG
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
	
	mov esp, 0x200000
	call MoveKernelto0x100000
	
	jmp CODESEG:0x100000

MoveKernelto0x100000:
	mov esi, 0x50000
	mov edi, 0x100000
	mov ecx, 16384
	
	cld
	rep movsd
	ret

print:
	lodsb
	mov ah, 0x0e
	int 0x10
	cmp al, 0
	jne print
	ret

;Disk Address Packet
dap:
	db 0x10
	db 0
	dw 128
	dw 0x0000
	dw 0x5000
	dq 10

GdtStart:
	dd 0x0, 0x0
GdtCodeDescriptor:
	dw 0xffff
	dw 0x0
	db 0x0
	db 10011010b
	db 11001111b
	db 0x0
GdtDataDescriptor:
	dw 0xffff
	dw 0x0
	db 0x0
	db 10010010b
	db 11001111b
	db 0x0
GdtEnd:

GDT:
	dw GdtEnd - GdtStart - 1
	dd GdtStart

CODESEG equ GdtCodeDescriptor - GdtStart
DATASEG equ GdtDataDescriptor - GdtStart

VBE_mode_info: times 256 db 0
vesa_failed: db "VESA Initialization Failed!", 0x0d, 0x0a, 0x0

stage2boot_drive: db 0
stage2_message: db "Successfully jumped to stage2!", 0x0d, 0x0a, 0x0
A20enabled: db "A20 Line enabled", 0x0d, 0x0a, 0x0
disk_error: db "Disk Read Error: Kernel initial read to 0x10000 failed!", 0x0d, 0x0a, 0x0
read1_success: db "Kernel initial read to 0x10000 successful!", 0x0d, 0x0a, 0x0
e820_success: db "Memory Map acquired successully!", 0x0d, 0x0a, 0x0
e820_error: db "E820 Failed!", 0x0d, 0x0a, 0x0
