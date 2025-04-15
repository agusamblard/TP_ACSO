; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0

section .data
empty_string db 0

.text

.global string_proc_list_create_asm
.global string_proc_node_create_asm
.global string_proc_list_add_node_asm
.global string_proc_list_concat_asm

.extern malloc
.extern free
.extern strlen
.extern strcpy
.extern strcat
.extern str_concat

// ===========================================================
// string_proc_list_create_asm
// Retorna un puntero a una lista nueva con first = last = NULL
// ===========================================================
string_proc_list_create_asm:
    stp x29, x30, [sp, -16]!     // guardar fp y lr
    mov x0, #16                  // tamaño de string_proc_list
    bl malloc
    cbz x0, .ret_null            // si malloc falla, retorna NULL
    mov x1, #0
    str x1, [x0, #0]             // list->first = NULL
    str x1, [x0, #8]             // list->last = NULL
    b .done

.ret_null:
    mov x0, #0

.done:
    ldp x29, x30, [sp], #16
    ret

// ===========================================================
// string_proc_node_create_asm(uint8_t type, char* hash)
// Retorna un puntero al nodo creado
// ===========================================================
string_proc_node_create_asm:
    stp x29, x30, [sp, -16]!     // guardar fp y lr
    mov x2, x0                   // type
    mov x3, x1                   // hash

    mov x0, #32                  // malloc(sizeof(node))
    bl malloc
    cbz x0, .ret_null_node

    mov x1, #0
    str x1, [x0, #0]             // next = NULL
    str x1, [x0, #8]             // previous = NULL
    strb w2, [x0, #16]           // type
    str x3, [x0, #24]            // hash

    b .done_node

.ret_null_node:
    mov x0, #0

.done_node:
    ldp x29, x30, [sp], #16
    ret

// ===========================================================
// string_proc_list_add_node_asm(list, type, hash)
// Agrega nodo al final de la lista
// ===========================================================
string_proc_list_add_node_asm:
    stp x29, x30, [sp, -16]!
    mov x3, x0              // guardar list
    mov x4, x1              // type
    mov x5, x2              // hash

    // Crear nodo
    mov x0, x4
    mov x1, x5
    bl string_proc_node_create_asm
    cbz x0, .done_add       // si malloc falla

    mov x6, x0              // nodo

    ldr x7, [x3, #8]        // list->last
    cbz x7, .empty_list

    // lista NO vacía: conectar
    str x7, [x6, #8]        // nodo->previous = list->last
    str x6, [x7, #0]        // list->last->next = nodo
    str x6, [x3, #8]        // list->last = nodo
    b .done_add

.empty_list:
    str x6, [x3, #0]        // list->first = nodo
    str x6, [x3, #8]        // list->last = nodo

.done_add:
    ldp x29, x30, [sp], #16
    ret

// ===========================================================
// char* string_proc_list_concat_asm(list, type, hash)
// Concatena a hash todos los nodos de list con tipo == type
// ===========================================================
string_proc_list_concat_asm:
    stp x29, x30, [sp, -16]!
    mov x3, x0      // list
    mov w4, w1      // type
    mov x5, x2      // hash

    // Copiar hash inicial
    bl strlen
    add x0, x0, #1
    bl malloc
    cbz x0, .ret_null_concat
    mov x6, x0          // resultado
    mov x0, x6
    mov x1, x5
    bl strcpy

    ldr x7, [x3]        // list->first

.loop_concat:
    cbz x7, .done_concat
    ldrb w8, [x7, #16]  // nodo->type
    cmp w8, w4
    b.ne .next_node

    ldr x0, [x7, #24]   // nodo->hash
    cbz x0, .next_node

    mov x1, x6
    bl str_concat
    cbz x0, .ret_null_concat
    mov x1, x6
    bl free
    mov x6, x0

.next_node:
    ldr x7, [x7, #0]    // nodo = nodo->next
    b .loop_concat

.done_concat:
    mov x0, x6
    b .done_concat_exit

.ret_null_concat:
    mov x0, #0

.done_concat_exit:
    ldp x29, x30, [sp], #16
    ret