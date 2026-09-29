ASM = nasm
QEMU = qemu-system-i386 -m 1G -vga std -no-reboot -no-shutdown -hda $(TARGET) -hdb $(FS)
CC = i686-elf-gcc
CFLAGS =  -m32 -ffreestanding -O0 -g -nostdlib -fno-pie -fno-pic -fomit-frame-pointer -fno-builtin -fno-stack-protector -Wall -Wextra -mno-mmx -mno-sse -I$(INCLUDE_DIR)
LD = i686-elf-ld 
MYLINKER = linker.ld
LDFLAGS = -m elf_i386 -T $(MYLINKER)

BUILD_DIR = build
SRC_DIR = src
BOOT_DIR = $(SRC_DIR)/bootloader
KERNEL_DIR = $(SRC_DIR)/kernel
TOOLS_DIR = $(SRC_DIR)/tools
INCLUDE_DIR = $(KERNEL_DIR)/include

STAGE1_SRC = $(BOOT_DIR)/stage1.asm
STAGE1_BIN = $(BUILD_DIR)/stage1.bin

STAGE2_SRC = $(BOOT_DIR)/stage2.asm
STAGE2_BIN = $(BUILD_DIR)/stage2.bin

KERNEL_ENTRY_SRC = $(BOOT_DIR)/kernel_entry.asm
KERNEL_ENTRY_OBJ = $(BUILD_DIR)/kernel_entry.o

KERNEL_C_MAIN = $(KERNEL_DIR)/kernel_main.c
KERNEL_OBJ_MAIN = $(BUILD_DIR)/kernel_main.o

SCREEN_C = $(KERNEL_DIR)/screen.c
SCREEN_OBJ = $(BUILD_DIR)/screen.o

IDT_ASM = $(KERNEL_DIR)/idt.asm
IDTASM_OBJ = $(BUILD_DIR)/idtasm.o
IDT_C = $(KERNEL_DIR)/idt.c
IDTC_OBJ = $(BUILD_DIR)/idt.o

KEYBOARD_C = $(KERNEL_DIR)/keyboard.c
KEYBOARD_OBJ = $(BUILD_DIR)/keyboard.o

PIT_C = $(KERNEL_DIR)/pit.c
PIT_OBJ = $(BUILD_DIR)/pit.o

UI_C = $(KERNEL_DIR)/ui.c
UI_OBJ = $(BUILD_DIR)/ui.o

MEM_MGR_C = $(KERNEL_DIR)/mem_mgr.c
MEM_MGR_OBJ = $(BUILD_DIR)/mem_mgr.o

TASK_MGR_C = $(KERNEL_DIR)/task_mgr.c
TASK_MGR_OBJ = $(BUILD_DIR)/task_mgr.o

DISK_MGR_C = $(KERNEL_DIR)/disk_mgr.c
DISK_MGR_OBJ = $(BUILD_DIR)/disk_mgr.o

STRING_C = $(KERNEL_DIR)/string.c
STRING_OBJ = $(BUILD_DIR)/string.o

SHELL_C = $(KERNEL_DIR)/shell.c
SHELL_OBJ = $(BUILD_DIR)/shell.o

FS_C = $(KERNEL_DIR)/fs.c
FS_OBJ = $(BUILD_DIR)/fs.o

MMAP_C = $(KERNEL_DIR)/mmap.c
MMAP_OBJ = $(BUILD_DIR)/mmap.o

RTC_C = $(KERNEL_DIR)/rtc.c
RTC_OBJ = $(BUILD_DIR)/rtc.o

LAUNCHER_C = $(KERNEL_DIR)/launcher.c
LAUNCHER_OBJ = $(BUILD_DIR)/launcher.o

BUILD_FS = $(TOOLS_DIR)/buildfs

SB = $(TOOLS_DIR)/sb.bin
ITBM= $(TOOLS_DIR)/itbm.bin
SBM = $(TOOLS_DIR)/sbm.bin
IT = $(TOOLS_DIR)/inodet.bin
ROOT = $(TOOLS_DIR)/root.bin

KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel.bin

OS_IMG = $(BUILD_DIR)/os.img
FS = fs.img

TARGET = $(OS_IMG)

all: $(BUILD_DIR) $(TARGET) 

$(BUILD_DIR): 
	mkdir -p $(BUILD_DIR)

$(STAGE1_BIN): $(STAGE1_SRC)
	$(ASM) -f bin $(STAGE1_SRC) -o $(STAGE1_BIN)

$(STAGE2_BIN): $(STAGE2_SRC)
	$(ASM) -f bin $(STAGE2_SRC) -o $(STAGE2_BIN)

$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY_SRC)
	$(ASM) -f elf32 -g -F dwarf $(KERNEL_ENTRY_SRC) -o $(KERNEL_ENTRY_OBJ)

$(KERNEL_OBJ_MAIN): $(KERNEL_C_MAIN)
	$(CC) $(CFLAGS) -c $(KERNEL_C_MAIN) -o $(KERNEL_OBJ_MAIN)

$(SCREEN_OBJ): $(SCREEN_C)
	$(CC) $(CFLAGS) -c $(SCREEN_C) -o $(SCREEN_OBJ)

$(IDTASM_OBJ): $(IDT_ASM)
	$(ASM) -f elf32 -g -F dwarf $(IDT_ASM) -o $(IDTASM_OBJ)

$(IDTC_OBJ): $(IDT_C)
	$(CC) $(CFLAGS) -c $(IDT_C) -o $(IDTC_OBJ)

$(KEYBOARD_OBJ): $(KEYBOARD_C)
	$(CC) $(CFLAGS) -c $(KEYBOARD_C) -o $(KEYBOARD_OBJ)

$(PIT_OBJ): $(PIT_C)
	$(CC) $(CFLAGS) -c $(PIT_C) -o $(PIT_OBJ)

$(UI_OBJ): $(UI_C)
	$(CC) $(CFLAGS) -c $(UI_C) -o $(UI_OBJ)

$(MEM_MGR_OBJ): $(MEM_MGR_C)
	$(CC) $(CFLAGS) -c $(MEM_MGR_C) -o $(MEM_MGR_OBJ)

$(TASK_MGR_OBJ): $(TASK_MGR_C)
	$(CC) $(CFLAGS) -c $(TASK_MGR_C) -o $(TASK_MGR_OBJ)

$(STRING_OBJ): $(STRING_C)
	$(CC) $(CFLAGS) -c $(STRING_C) -o $(STRING_OBJ)

$(DISK_MGR_OBJ): $(DISK_MGR_C)
	$(CC) $(CFLAGS) -c $(DISK_MGR_C) -o $(DISK_MGR_OBJ)

$(SHELL_OBJ): $(SHELL_C)
	$(CC) $(CFLAGS) -c $(SHELL_C) -o $(SHELL_OBJ)

$(FS_OBJ): $(FS_C)
	$(CC) $(CFLAGS) -c $(FS_C) -o $(FS_OBJ)

$(MMAP_OBJ): $(MMAP_C)
	$(CC) $(CFLAGS) -c $(MMAP_C) -o $(MMAP_OBJ)

$(RTC_OBJ): $(RTC_C)
	$(CC) $(CFLAGS) -c $(RTC_C) -o $(RTC_OBJ)

$(LAUNCHER_OBJ): $(LAUNCHER_C)
	$(CC) $(CFLAGS) -c $(LAUNCHER_C) -o $(LAUNCHER_OBJ)

$(KERNEL_ELF): $(KERNEL_ENTRY_OBJ) $(KERNEL_OBJ_MAIN) $(SCREEN_OBJ) $(IDTC_OBJ) $(IDTASM_OBJ) $(KEYBOARD_OBJ) $(PIT_OBJ) $(UI_OBJ) $(MEM_MGR_OBJ) $(TASK_MGR_OBJ) $(STRING_OBJ) $(DISK_MGR_OBJ) $(SHELL_OBJ) $(FS_OBJ) $(MMAP_OBJ) $(RTC_OBJ) $(LAUNCHER_OBJ) $(MYLINKER)
	$(LD) $(LDFLAGS) $(KERNEL_ENTRY_OBJ) $(KERNEL_OBJ_MAIN) $(SCREEN_OBJ) $(IDTC_OBJ) $(IDTASM_OBJ) $(KEYBOARD_OBJ) $(PIT_OBJ) $(UI_OBJ) $(MEM_MGR_OBJ) $(TASK_MGR_OBJ) $(STRING_OBJ) $(DISK_MGR_OBJ) $(SHELL_OBJ) $(FS_OBJ) $(MMAP_OBJ) $(RTC_OBJ) $(LAUNCHER_OBJ) -o $(KERNEL_ELF)

$(KERNEL_BIN): $(KERNEL_ELF)
	objcopy -O binary $(KERNEL_ELF) $(KERNEL_BIN)

$(OS_IMG): $(STAGE1_BIN) $(STAGE2_BIN) $(KERNEL_BIN)
	dd if=/dev/zero of=$(OS_IMG) bs=512 count=2880
	dd if=$(STAGE1_BIN) of=$(OS_IMG) bs=512 conv=notrunc seek=0
	dd if=$(STAGE2_BIN) of=$(OS_IMG) bs=512 conv=notrunc seek=1
	dd if=$(KERNEL_BIN) of=$(OS_IMG) bs=512 conv=notrunc seek=10 
	echo "==> Final os img constructed successfully!"

fs:
	dd if=/dev/zero of=$(FS) bs=1M count=10
	dd if=$(SB) of=$(FS) bs=512 conv=notrunc seek=0
	dd if=$(IT) of=$(FS) bs=512 conv=notrunc seek=1
	dd if=$(SBM) of=$(FS) bs=512 conv=notrunc seek=131
	dd if=$(ITBM) of=$(FS) bs=512 conv=notrunc seek=135
	dd if=$(ROOT) of=$(FS) bs=512 conv=notrunc seek=137

run:
	$(QEMU)
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean run
