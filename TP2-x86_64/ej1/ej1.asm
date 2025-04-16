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

extern malloc
extern free
extern str_concat
extern strlen
extern strcpy
extern strcat
extern fprintf

;---------------------------
; string_proc_list_add_node_asm
;-------------------------------
string_proc_list_create_asm:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 16
    mov     edi, 16
    call    malloc
    mov     [rbp-8], rax
    cmp     qword [rbp-8], 0
    jne     .init_list_fields
    mov     eax, 0
    jmp     .end_list_create
.init_list_fields:
    mov     rax, [rbp-8]
    mov     qword [rax], 0
    mov     rax, [rbp-8]
    mov     qword [rax+8], 0
    mov     rax, [rbp-8]
.end_list_create:
    leave
    ret

;---------------------------
; string_proc_list_create_asm
;-------------------------------
string_proc_node_create_asm:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 32
    mov     eax, edi
    mov     [rbp-32], rsi
    mov     [rbp-20], al
    mov     edi, 32
    call    malloc
    mov     [rbp-8], rax
    cmp     qword [rbp-8], 0
    jne     .init_node_fields
    mov     eax, 0
    jmp     .exit_add_node
.init_node_fields:
    mov     rax, [rbp-8]
    mov     qword [rax], 0
    mov     rax, [rbp-8]
    mov     qword [rax+8], 0
    mov     rax, [rbp-8]
    movzx   edx, byte [rbp-20]
    mov     [rax+16], dl
    mov     rax, [rbp-8]
    mov     rdx, [rbp-32]
    mov     [rax+24], rdx
    mov     rax, [rbp-8]
.exit_add_node:
    leave
    ret

;-------------------------------
; string_proc_list_add_node_asm
;-------------------------------
string_proc_list_add_node_asm:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 48
    mov     [rbp-24], rdi
    mov     eax, esi
    mov     [rbp-40], rdx
    mov     [rbp-28], al
    movzx   eax, byte [rbp-28]
    mov     rdx, [rbp-40]
    mov     rsi, rdx
    mov     edi, eax
    call    string_proc_node_create_asm
    mov     [rbp-8], rax
    cmp     qword [rbp-8], 0
    je      .node_creation_failed
    mov     rax, [rbp-24]
    mov     rax, [rax]
    test    rax, rax
    jne     .add_to_end
    mov     rax, [rbp-24]
    mov     rdx, [rbp-8]
    mov     [rax], rdx
    mov     rax, [rbp-24]
    mov     rdx, [rbp-8]
    mov     [rax+8], rdx
    jmp     .end_list_add_node
.add_to_end:
    mov     rax, [rbp-24]
    mov     rax, [rax+8]
    mov     rdx, [rbp-8]
    mov     [rax], rdx
    mov     rax, [rbp-24]
    mov     rdx, [rax+8]
    mov     rax, [rbp-8]
    mov     [rax+8], rdx
    mov     rax, [rbp-24]
    mov     rdx, [rbp-8]
    mov     [rax+8], rdx
    jmp     .end_list_add_node
.node_creation_failed:
    nop
.end_list_add_node:
    leave
    ret

;-------------------------------
; string_proc_list_concat_asm
;-------------------------------
string_proc_list_concat_asm:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 64
    mov     [rbp-40], rdi
    mov     eax, esi
    mov     [rbp-56], rdx
    mov     [rbp-44], al
    mov     rax, [rbp-40]
    mov     rax, [rax]
    mov     [rbp-8], rax
    mov     qword [rbp-16], 0
    jmp     .loop_check
.loop_body:
    mov     rax, [rbp-8]
    movzx   eax, byte [rax+16]
    cmp     byte [rbp-44], al
    jne     .advance_to_next
    cmp     qword [rbp-16], 0
    jne     .concat_and_free_prev
    mov     rax, [rbp-8]
    mov     rdx, [rax+24]
    mov     rax, [rbp-56]
    mov     rsi, rdx
    mov     rdi, rax
    call    str_concat
    mov     [rbp-16], rax
    jmp     .advance_to_next
.concat_and_free_prev:
    mov     rax, [rbp-8]
    mov     rdx, [rax+24]
    mov     rax, [rbp-16]
    mov     rsi, rdx
    mov     rdi, rax
    call    str_concat
    mov     [rbp-24], rax
    mov     rax, [rbp-16]
    mov     rdi, rax
    call    free
    mov     rax, [rbp-24]
    mov     [rbp-16], rax
.advance_to_next:
    mov     rax, [rbp-8]
    mov     rax, [rax]
    mov     [rbp-8], rax
.loop_check:
    cmp     qword [rbp-8], 0
    jne     .loop_body
    mov     rax, [rbp-16]
    leave
    ret