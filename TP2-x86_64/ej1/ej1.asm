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

; ---------------------------------------------
; string_proc_list_create_asm()
; Retorna un puntero a una lista vacía
; ---------------------------------------------
string_proc_list_create_asm:
    mov edi, 16
    call malloc
    test rax, rax
    je .return_null
    mov qword [rax], 0
    mov qword [rax + 8], 0
    ret
.return_null:
    xor eax, eax
    ret

; ---------------------------------------------
; string_proc_node_create_asm(uint8_t type, char* hash)
; ---------------------------------------------
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

; ---------------------------------------------
; string_proc_list_add_node_asm(string_proc_list* list, uint8_t type, char* hash)
; ---------------------------------------------
string_proc_list_add_node_asm:
    test rdi, rdi
    je .done
    push rbx
    mov rbx, rdi            ; backup de list
    movzx edi, sil          ; type → edi
    mov rsi, rdx            ; hash
    call string_proc_node_create_asm
    test rax, rax
    je .restore
    mov rcx, [rbx + 8]      ; list->last
    test rcx, rcx
    je .first_node
    mov [rax + 8], rcx      ; node->previous = last
    mov [rcx], rax          ; last->next = node
    mov [rbx + 8], rax      ; list->last = node
    jmp .restore
.first_node:
    mov [rbx], rax
    mov [rbx + 8], rax
.restore:
    pop rbx
.done:
    ret

; ---------------------------------------------
; string_proc_list_concat_asm(string_proc_list* list, uint8_t type, char* hash)
; ---------------------------------------------
string_proc_list_concat_asm:
    test rdi, rdi
    je .return_hash_copy
    test rdx, rdx
    je .return_hash_copy

    push rbx
    push rsi
    mov dl, sil             ; guardar tipo en dl
    mov rdi, empty_string
    mov rsi, rdx            ; hash
    call str_concat
    mov rbx, rax            ; resultado parcial
    pop rsi
    pop rbx

    mov rcx, [rdi]          ; nodo actual = list->first
.loop:
    test rcx, rcx
    je .done
    mov al, byte [rcx + 16] ; nodo->type
    cmp al, dl              ; comparar con tipo guardado
    jne .next
    mov rdi, rbx
    mov rsi, [rcx + 24]     ; nodo->hash
    call str_concat
    mov rdi, rbx
    call free
    mov rbx, rax
.next:
    mov rcx, [rcx]          ; nodo = nodo->next
    jmp .loop
.done:
    mov rax, rbx
    ret

.return_hash_copy:
    mov rdi, empty_string
    mov rsi, rdx
    call str_concat
    ret
