#include "ej1.h"
#include <string.h>



string_proc_list* string_proc_list_create(void) {
    string_proc_list* lista = malloc(sizeof(string_proc_list));
    if (lista == NULL) {
        return NULL;
    }
    lista->first = NULL;
    lista->last = NULL;
    return lista;
}

string_proc_node* string_proc_node_create(uint8_t type, char* hash) {
    string_proc_node* nodo = malloc(sizeof(string_proc_node));
    if (nodo == NULL) return NULL;

    nodo->type = type;
    nodo->hash = hash;  
    nodo->next = NULL;
    nodo->previous = NULL;
    return nodo;
}

void string_proc_list_add_node(string_proc_list* list, uint8_t type, char* hash) {
    if (list == NULL) return;

    string_proc_node* nodo = string_proc_node_create(type, hash);
    if (nodo == NULL) {
        return;
    }

    if (list->last == NULL) {
        list->first = nodo;
        list->last = nodo;
    } else {
        nodo->previous = list->last;
        list->last->next = nodo;
        list->last = nodo;
    }
}

// Concatena todos los hash de nodos del tipo especificado
char* string_proc_list_concat(string_proc_list* list, uint8_t type, char* hash) {
    if (list == NULL || hash == NULL) return NULL;

    char* resultado = malloc(strlen(hash) + 1);
    if (resultado == NULL) return NULL;
    
	strcpy(resultado, hash);

    string_proc_node* actual = list->first;
    while (actual != NULL) {
        if (actual->type == type && actual->hash != NULL) {
            char* resultado_nuevo = str_concat(resultado, actual->hash);
            free(resultado);
            if (resultado_nuevo == NULL) return NULL;
            resultado = resultado_nuevo;
        }
        actual = actual->next;
    }

    return resultado;
}


/** AUX FUNCTIONS **/

void string_proc_list_destroy(string_proc_list* list){
	/* borro los nodos: */
	string_proc_node* current_node	= list->first;
	string_proc_node* next_node		= NULL;
	while(current_node != NULL){
		next_node = current_node->next;
		string_proc_node_destroy(current_node);
		current_node	= next_node;
	}
	/*borro la lista:*/
	list->first = NULL;
	list->last  = NULL;
	free(list);
}
void string_proc_node_destroy(string_proc_node* node){

	node->next      = NULL;
	node->previous	= NULL;
	node->hash		= NULL;
	node->type      = 0;			
	free(node);
}


char* str_concat(char* a, char* b) {
	int len1 = strlen(a);
    int len2 = strlen(b);
	int totalLength = len1 + len2;
    char *result = (char *)malloc(totalLength + 1); 
    strcpy(result, a);
    strcat(result, b);
    return result;  
}

void string_proc_list_print(string_proc_list* list, FILE* file){
        uint32_t length = 0;
        string_proc_node* current_node  = list->first;
        while(current_node != NULL){
                length++;
                current_node = current_node->next;
        }
        fprintf( file, "List length: %d\n", length );
		current_node    = list->first;
        while(current_node != NULL){
                fprintf(file, "\tnode hash: %s | type: %d\n", current_node->hash, current_node->type);
                current_node = current_node->next;
        }
}