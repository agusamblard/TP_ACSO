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

; ==============================================
; string_proc_list* string_proc_list_create_asm()
; ==============================================
string_proc_list_create_asm:
    ; reservar memoria para la estructura (16 bytes)
    mov rdi, 16
    call malloc
    test rax, rax
    je .return_null

    ; inicializar los campos first y last en NULL
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
    ; reservar memoria para la estructura (32 bytes)
    mov rdi, 32
    call malloc
    test rax, rax
    je .return_null_node

    ; rdi = type, rsi = hash (ver orden de parámetros AMD64 System V)
    mov rdx, rdi            ; guardar type en rdx
    mov rcx, rsi            ; guardar hash en rcx

    ; inicializar campos del nodo
    mov qword [rax], 0          ; next = NULL
    mov qword [rax + 8], 0      ; previous = NULL
    mov byte [rax + 16], dl     ; type (uint8_t) en offset 16
    mov qword [rax + 24], rcx   ; hash (puntero) en offset 24

    ret

.return_null_node:
    mov rax, 0
    ret

; ====================================================
; void string_proc_list_add_node_asm(string_proc_list* list, uint8_t type, char* hash)
; ====================================================
string_proc_list_add_node_asm:
    ; list = rdi, type = sil (parte baja de rsi), hash = rdx

    ; preparar argumentos para crear el nodo
    movzx edi, sil          ; type en edi
    mov rsi, rdx            ; hash en rsi
    call string_proc_node_create_asm
    test rax, rax
    je .end_add_node

    ; rdi todavía tiene list
    mov rbx, rdi            ; rbx = list
    mov rcx, [rbx]          ; list->first
    test rcx, rcx
    je .add_first_node

    ; agregar al final de la lista existente
    mov rcx, [rbx + 8]      ; rcx = list->last
    mov [rcx], rax          ; last->next = new_node
    mov [rax + 8], rcx      ; new_node->previous = last
    mov [rbx + 8], rax      ; list->last = new_node
    jmp .end_add_node

.add_first_node:
    mov [rbx], rax          ; list->first = new_node
    mov [rbx + 8], rax      ; list->last = new_node

.end_add_node:
    ret

; ===================================================
; char* string_proc_list_concat_asm(string_proc_list* list, uint8_t type, char* hash)
; ===================================================
string_proc_list_concat_asm:
    ; rdi = list, sil = type, rdx = hash

    ; hacer strdup inicial (simulate en C con str_concat y "")
    mov rdi, rdx
    call strdup
    mov rbx, rax            ; rbx = result

    ; recorrer la lista
    mov rcx, [rdi]          ; rcx = list->first

.loop_concat:
    test rcx, rcx
    je .done_concat

    ; comparar tipo
    mov al, byte [rcx + 16] ; tipo del nodo
    cmp al, sil
    jne .next_node

    ; concatenar
    mov rdi, rbx            ; resultado actual
    mov rsi, [rcx + 24]     ; nodo->hash
    call str_concat         ; str_concat(result, nodo->hash)
    mov rdi, rbx            ; liberar string anterior
    call free
    mov rbx, rax            ; nuevo resultado

.next_node:
    mov rcx, [rcx]          ; ir al siguiente nodo
    jmp .loop_concat

.done_concat:
    mov rax, rbx
    ret
