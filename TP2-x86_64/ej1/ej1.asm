; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0

section .data

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

; FUNCIONES auxiliares que pueden llegar a necesitar:
extern malloc
extern free
extern str_concat


string_proc_list_create_asm:
    ; Stackframe
    push rbp
    mov rbp, rsp

    ; malloc(sizeof(string_proc_list)) -> malloc(16)
    mov edi, 16
    call malloc
    ; rax tiene el puntero devuelto

    ; if rax == NULL -> salto
    test rax, rax
    je .return_null

    ; *(rax) = 0 (first = NULL)
    mov qword [rax], 0

    ; *(rax + 8) = 0 (last = NULL)
    mov qword [rax + 8], 0

.return_null:
    ; rax ya tiene el puntero (sea NULL o válido)
    pop rbp
    ret

string_proc_node_create_asm:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 32
        
        ; Guardar parámetros
        mov     byte [rbp-20], dil  ; type (uint8_t)
        mov     qword [rbp-32], rsi ; hash (const char*)
        
        ; Asignar memoria
        mov     edi, 32
        call    malloc
        mov     qword [rbp-8], rax  ; Guardar ptr devuelto
        
        ; Verificar malloc
        cmp     qword [rbp-8], 0
        je      .return_null
        
        ; Inicializar estructura
        mov     rax, qword [rbp-8]
        mov     qword [rax], 0       ; next = NULL
        mov     qword [rax+8], 0     ; previous = NULL
        
        ; Asignar type 
        movzx   edx, byte [rbp-20]   ; Cargar type correctamente
        mov     byte [rax+16], dl
        
        ; Asignar hash
        mov     rdx, qword [rbp-32]
        mov     qword [rax+24], rdx
        
        mov     rax, qword [rbp-8]   ; Devolver ptr
        jmp     .return
        
.return_null:
        xor     eax, eax                 ; Devolver NULL
        
.return:
        leave
        ret


string_proc_list_add_node_asm:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 48

    mov     qword [rbp-24], rdi       ; list
    mov     eax, esi                  ; type
    mov     qword [rbp-40], rdx       ; string pointer
    mov     byte [rbp-28], al

    cmp     qword [rbp-24], 0
    je      .L9

    movzx   eax, byte [rbp-28]
    mov     rdx, qword [rbp-40]
    mov     rsi, rdx
    mov     edi, eax
    call    string_proc_node_create_asm
    mov     qword [rbp-8], rax

    cmp     qword [rbp-8], 0
    je      .L10

    mov     rax, qword [rbp-24]
    mov     rax, qword [rax+8]
    test    rax, rax
    jne     .L8

    mov     rax, qword [rbp-24]
    mov     rdx, qword [rbp-8]
    mov     qword [rax], rdx

    mov     rax, qword [rbp-24]
    mov     rdx, qword [rbp-8]
    mov     qword [rax+8], rdx

    jmp     .L4

.L8:
    mov     rax, qword [rbp-24]
    mov     rdx, qword [rax+8]

    mov     rax, qword [rbp-8]
    mov     qword [rax+8], rdx

    mov     rax, qword [rbp-24]
    mov     rax, qword [rax+8]

    mov     rdx, qword [rbp-8]
    mov     qword [rax], rdx

    mov     rax, qword [rbp-24]
    mov     rdx, qword [rbp-8]
    mov     qword [rax+8], rdx

    jmp     .L4

.L9:
    nop
    jmp     .L4

.L10:
    nop

.L4:
    leave
    ret

string_proc_list_concat_asm:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 64

    mov     qword [rbp-40], rdi     ; list
    mov     eax, esi                ; type
    mov     qword [rbp-56], rdx     ; string
    mov     byte [rbp-44], al

    cmp     qword [rbp-40], 0
    je      .L4
    cmp     qword [rbp-56], 0
    jne     .L5

.L4:
    mov     eax, 0
    jmp     .L6

.L5:
    mov     rax, qword [rbp-56]
    mov     rdi, rax
    call    strlen_custom
    add     rax, 1
    mov     rdi, rax
    call    malloc
    mov     qword [rbp-8], rax

    cmp     qword [rbp-8], 0
    jne     .L7

    mov     eax, 0
    jmp     .L6

.L7:
    mov     rdx, qword [rbp-56]
    mov     rax, qword [rbp-8]
    mov     rsi, rdx
    mov     rdi, rax
    call    strcpy_custom

    mov     rax, qword [rbp-40]
    mov     rax, qword [rax]
    mov     qword [rbp-16], rax

    jmp     .L8

.L11:
    mov     rax, qword [rbp-16]
    movzx   eax, byte [rax+16]
    cmp     byte [rbp-44], al
    jne     .L9

    mov     rax, qword [rbp-16]
    mov     rax, qword [rax+24]
    test    rax, rax
    je      .L9

    mov     rax, qword [rbp-16]
    mov     rdx, qword [rax+24]
    mov     rax, qword [rbp-8]
    mov     rsi, rdx
    mov     rdi, rax
    call    str_concat
    mov     qword [rbp-24], rax

    mov     rax, qword [rbp-8]
    mov     rdi, rax
    call    free

    cmp     qword [rbp-24], 0
    jne     .L10

    mov     eax, 0
    jmp     .L6

.L10:
    mov     rax, qword [rbp-24]
    mov     qword [rbp-8], rax

.L9:
    mov     rax, qword [rbp-16]
    mov     rax, qword [rax]
    mov     qword [rbp-16], rax

.L8:
    cmp     qword [rbp-16], 0
    jne     .L11

    mov     rax, qword [rbp-8]

.L6:
    leave
    ret

strcpy_custom:
    mov     rax, rdi         ; guardar el puntero original de destino
.copy:
    mov     bl, [rsi]        ; cargar byte origen
    mov     [rdi], bl        ; copiar a destino
    inc     rsi
    inc     rdi
    test    bl, bl           ; terminamos si es 0
    jnz     .copy
    ret

strlen_custom:
    xor     rax, rax            ; contador = 0
.next_char:
    cmp     BYTE [rdi + rax], 0
    je      .done
    inc     rax
    jmp     .next_char
.done:
    ret