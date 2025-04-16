; === Defines ===
%define NULL 0

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm
global string_proc_node_destroy_asm
global string_proc_list_destroy_asm

extern malloc
extern free
extern str_concat

; =========================================================
; string_proc_list_create_asm
; =========================================================
string_proc_list_create_asm:
    mov rdi, 16
    call malloc
    test rax, rax
    jz .fail

    mov qword [rax], NULL        ; list->first
    mov qword [rax + 8], NULL    ; list->last
    ret

.fail:
    xor rax, rax
    ret

; =========================================================
; string_proc_node_create_asm
; =========================================================
string_proc_node_create_asm:
    ; rdi = type (uint8)
    ; rsi = hash (char*)

    mov rbx, rdi        ; guardar type en bl
    mov rcx, rsi        ; guardar hash en rcx

    mov rdi, 32
    call malloc
    test rax, rax
    jz .fail_create

    mov rdx, rax        ; rdx = nuevo nodo
    mov qword [rdx], 0        ; next
    mov qword [rdx + 8], 0    ; previous
    mov byte [rdx + 16], bl   ; type
    mov qword [rdx + 24], rcx ; hash

    mov rax, rdx
    ret

.fail_create:
    xor rax, rax
    ret

; =========================================================
; string_proc_list_add_node_asm
; =========================================================
string_proc_list_add_node_asm:
    ; rdi = list
    ; rsi = type
    ; rdx = hash

    push rbp
    mov rbp, rsp

    test rdi, rdi
    jz .error

    mov r8, rsi
    mov r9, rdx
    mov rsi, r8
    mov rdx, r9
    call string_proc_node_create_asm
    test rax, rax
    jz .done

    mov r10, rax              ; new node
    mov r11, [rdi + 8]        ; current tail

    test r11, r11
    jz .empty_list

    mov [r11], r10            ; tail->next = new
    mov [r10 + 8], r11        ; new->previous = tail
    mov [rdi + 8], r10        ; list->last = new
    jmp .done

.empty_list:
    mov [rdi], r10            ; list->first = new
    mov [rdi + 8], r10        ; list->last = new

.error:
    pop rbp
    ret

.done:
    pop rbp
    ret

; =========================================================
; string_proc_list_concat_asm
; =========================================================
string_proc_list_concat_asm:
    ; rdi = list
    ; rsi = type
    ; rdx = initial hash

    push rbp
    mov rbp, rsp

    test rdi, rdi
    jz .return
    test rdx, rdx
    jz .return

    mov r8, [rdi]             ; current = list->first
.loop:
    test r8, r8
    jz .return

    movzx r9, byte [r8 + 16]  ; node->type
    cmp r9b, sil
    jne .next

    mov rdi, rdx              ; accumulated
    mov rsi, [r8 + 24]        ; node->hash
    call str_concat
    mov rdx, rax              ; update accumulator

.next:
    mov r8, [r8]              ; current = current->next
    jmp .loop

.return:
    mov rax, rdx
    pop rbp
    ret

; =========================================================
; string_proc_node_destroy_asm
; =========================================================
string_proc_node_destroy_asm:
    ; rdi = node
    test rdi, rdi
    jz .done
    call free
.done:
    ret

; =========================================================
; string_proc_list_destroy_asm
; =========================================================
string_proc_list_destroy_asm:
    ; rdi = list
    push rbp
    mov rbp, rsp

    test rdi, rdi
    jz .end

    mov rbx, [rdi]  ; current = list->first

.loop:
    test rbx, rbx
    jz .free_list

    mov rdx, [rbx]       ; next = current->next
    mov rsi, rbx         ; arg to free
    call free
    mov rbx, rdx         ; current = next
    jmp .loop

.free_list:
    mov rsi, rdi
    call free

.end:
    pop rbp
    ret
