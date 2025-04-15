; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0

section .data
empty_string db 0

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

extern malloc
extern free
extern str_concat

; string_proc_list* string_proc_list_create_asm()
string_proc_list_create_asm:
    mov edi, 16
    call malloc
    test rax, rax
    je .return_null
    mov qword [rax], 0
    mov qword [rax + 8], 0
.return_null:
    ret

; string_proc_node* string_proc_node_create_asm(uint8_t type, char* hash)
string_proc_node_create_asm:
    mov edi, 32
    call malloc
    test rax, rax
    je .return_null_node
    mov qword [rax], 0
    mov qword [rax + 8], 0
    movzx edx, dil
    mov byte [rax + 16], dl
    mov qword [rax + 24], rsi
    ret
.return_null_node:
    xor eax, eax
    ret

; void string_proc_list_add_node_asm(string_proc_list* list, uint8_t type, char* hash)
string_proc_list_add_node_asm:
    test rdi, rdi
    je .done
    movzx edi, sil
    call string_proc_node_create_asm
    test rax, rax
    je .done
    mov rcx, [rdi + 8]
    test rcx, rcx
    je .first_node
    mov [rax + 8], rcx
    mov [rcx], rax
    mov [rdi + 8], rax
    jmp .done
.first_node:
    mov [rdi], rax
    mov [rdi + 8], rax
.done:
    ret

; char* string_proc_list_concat_asm(string_proc_list* list, uint8_t type, char* hash)
string_proc_list_concat_asm:
    test rdi, rdi
    je .return_hash_copy
    test rdx, rdx
    je .return_hash_copy
    push rdi
    mov rdi, empty_string
    mov rsi, rdx
    call str_concat
    mov rbx, rax
    pop rdi
    mov rcx, [rdi]
.loop:
    test rcx, rcx
    je .done
    mov al, byte [rcx + 16]
    cmp al, sil
    jne .next
    mov rdi, rbx
    mov rsi, [rcx + 24]
    call str_concat
    mov rdi, rbx
    call free
    mov rbx, rax
.next:
    mov rcx, [rcx]
    jmp .loop
.done:
    mov rax, rbx
    ret
.return_hash_copy:
    mov rdi, empty_string
    mov rsi, rdx
    call str_concat
    ret
