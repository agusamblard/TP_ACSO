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

; ==============================================
; string_proc_list* string_proc_list_create_asm()
; ==============================================
string_proc_list_create_asm:
    mov rdi, 16          ; sizeof(string_proc_list)
    call malloc
    test rax, rax
    je .return_null

    mov qword [rax], 0       ; first = NULL
    mov qword [rax + 8], 0   ; last = NULL
    ret

.return_null:
    mov rax, 0
    ret

; ===================================================
; string_proc_node* string_proc_node_create_asm(uint8_t type, char* hash)
; ===================================================
string_proc_node_create_asm:
    mov rdi, 32          ; sizeof(string_proc_node)
    call malloc
    test rax, rax
    je .return_null_node

    mov rdx, rdi         ; rdx = type
    mov rcx, rsi         ; rcx = hash

    mov qword [rax], 0          ; next = NULL
    mov qword [rax + 8], 0      ; previous = NULL
    mov byte [rax + 16], dl     ; type
    mov qword [rax + 24], rcx   ; hash
    ret

.return_null_node:
    mov rax, 0
    ret

; ====================================================
; void string_proc_list_add_node_asm(string_proc_list* list, uint8_t type, char* hash)
; ====================================================
string_proc_list_add_node_asm:
    movzx edi, sil          ; type → edi
    mov rsi, rdx            ; hash → rsi
    call string_proc_node_create_asm
    test rax, rax
    je .end_add_node

    mov rbx, rdi            ; list
    mov rcx, [rbx]          ; list->first
    test rcx, rcx
    je .add_first_node

    ; agregar al final
    mov rcx, [rbx + 8]      ; list->last
    mov [rcx], rax          ; last->next = node
    mov [rax + 8], rcx      ; node->previous = last
    mov [rbx + 8], rax      ; list->last = node
    jmp .end_add_node

.add_first_node:
    mov [rbx], rax          ; list->first = node
    mov [rbx + 8], rax      ; list->last = node

.end_add_node:
    ret

; ===================================================
; char* string_proc_list_concat_asm(string_proc_list* list, uint8_t type, char* hash)
; ===================================================
string_proc_list_concat_asm:
    ; rdi = list, sil = type, rdx = hash

    ; inicializar resultado con str_concat("", hash)
    mov rdi, empty_string
    mov rsi, rdx
    call str_concat
    mov rbx, rax            ; rbx = result

    mov rcx, [rdi]          ; rcx = list->first

.loop_concat:
    test rcx, rcx
    je .done_concat

    mov al, byte [rcx + 16] ; tipo actual
    cmp al, sil
    jne .next_node

    ; concatenar si coincide tipo
    mov rdi, rbx
    mov rsi, [rcx + 24]     ; nodo->hash
    call str_concat
    mov rdi, rbx
    call free
    mov rbx, rax            ; nuevo result

.next_node:
    mov rcx, [rcx]          ; next
    jmp .loop_concat

.done_concat:
    mov rax, rbx
    ret
