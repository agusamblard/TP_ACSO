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

; === string_proc_list_create_asm() ===
string_proc_list_create_asm:
    mov edi, 16
    call malloc
    test rax, rax
    je .return_null_list
    mov qword [rax], 0         ; first = NULL
    mov qword [rax + 8], 0     ; last = NULL
    ret
.return_null_list:
    xor eax, eax
    ret

; === string_proc_node_create_asm(uint8_t type, char* hash) ===
string_proc_node_create_asm:
    mov edi, 32
    call malloc
    test rax, rax
    je .return_null_node
    mov qword [rax], 0          ; next = NULL
    mov qword [rax + 8], 0      ; previous = NULL
    movzx edx, dil              ; type → edx
    mov byte [rax + 16], dl
    mov qword [rax + 24], rsi   ; hash
    ret
.return_null_node:
    xor eax, eax
    ret

; === string_proc_list_add_node_asm(list, type, hash) ===
string_proc_list_add_node_asm:
    test rdi, rdi
    je .done_add_node

    push rbx
    mov rbx, rdi                ; backup list
    movzx edi, sil              ; type → edi
    mov rsi, rdx                ; hash
    call string_proc_node_create_asm
    test rax, rax
    je .restore_add_node

    mov rcx, [rbx + 8]          ; list->last
    test rcx, rcx
    je .first_node

    ; caso general
    mov [rax + 8], rcx          ; node->previous = last
    mov [rcx], rax              ; last->next = node
    mov [rbx + 8], rax          ; list->last = node
    jmp .restore_add_node

.first_node:
    mov [rbx], rax              ; list->first = node
    mov [rbx + 8], rax          ; list->last = node

.restore_add_node:
    pop rbx
.done_add_node:
    ret

; === string_proc_list_concat_asm(list, type, hash) ===
string_proc_list_concat_asm:
    test rdi, rdi
    je .return_hash_copy
    test rdx, rdx
    je .return_hash_copy

    push rbx
    push rsi

    mov r8, rdi                 ; guardar list
    mov dl, sil                ; guardar type

    ; resultado = str_concat("", hash)
    mov rdi, empty_string
    mov rsi, rdx
    call str_concat
    mov rbx, rax                ; resultado parcial

    pop rsi
    pop rbx

    mov rcx, [r8]               ; rcx = list->first

.loop:
    test rcx, rcx
    je .done_concat

    ; Validar nodo->hash no nulo antes de seguir
    mov rsi, [rcx + 24]
    test rsi, rsi
    je .next_node

    mov al, byte [rcx + 16]     ; nodo->type
    cmp al, dl
    jne .next_node

    mov rdi, rbx
    call str_concat
    test rax, rax
    je .next_node
    mov rdi, rbx
    call free
    mov rbx, rax

.next_node:
    mov rcx, [rcx]
    jmp .loop

.done_concat:
    mov rax, rbx
    ret

.return_hash_copy:
    mov rdi, empty_string
    mov rsi, rdx
    call str_concat
    ret
