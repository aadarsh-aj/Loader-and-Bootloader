bits 16
org 0x7c00

start:
    ; The BIOS loads this boot sector at 0x7C00.
    ; Make SI point to the first character of `message`.
    ; Hint: `lodsb` will use SI.
    mov si, message


print:
    ; Read the next character from the string.
    ; After this instruction, AL should contain the character.
    lodsb

    ; Check whether we reached the end of the string.
    cmp al, 0
    je hang

    ; BIOS video service:
    ; AH = 0x0E means "display the character in AL".
    mov ah, 0x0e

    ; Call the BIOS video service.
    int 0x10

    ; Go back and process the next character.
    jmp print


hang:
    ; We are finished printing.
    ; Disable interrupts.
    cli

    ; Halt the CPU.
    hlt

    ; Stay here forever (in case an NMI wakes it back up from hlt).
    jmp hang


message:
    db "Hello from my bootloader!", 0


; A boot sector must be exactly 512 bytes.
; Fill the unused space with zeroes.
times 510-($-$$) db 0


; Add the boot-sector signature.
dw 0xaa55
