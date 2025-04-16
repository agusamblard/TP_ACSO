#include "ej1.h"

string_proc_list* string_proc_list_create(void){
	string_proc_list* list = (string_proc_list*)malloc(sizeof(string_proc_list));
	if(list == NULL){
		fprintf(stderr, "Error: No se pudo crear la lista de nodos.\n");
		return NULL;
	}
	list->first = NULL;
	list->last  = NULL;
	return list;
}

string_proc_node* string_proc_node_create(uint8_t type, char* hash){
	string_proc_node* node = (string_proc_node*)malloc(sizeof(string_proc_node));
	if(node == NULL){
		fprintf(stderr, "Error: No se pudo crear el nodo.\n");
		return NULL;
	}
	node->next      = NULL;
	node->previous  = NULL;
	node->hash      = hash;
	node->type      = type;			
	return node;
}

void string_proc_list_add_node(string_proc_list* list, uint8_t type, char* hash){
	string_proc_node* node = string_proc_node_create(type, hash);
	if(node == NULL){
		fprintf(stderr, "Error: No se pudo crear el nodo.\n");
		return;
	}
	if(list->first == NULL){
		list->first = node;
		list->last  = node;
	}else{
		list->last->next = node;
		node->previous   = list->last;
		list->last      = node;
	}
}

char* string_proc_list_concat(string_proc_list* list, uint8_t type, char* hash) {
    if (list == NULL || hash == NULL) {
        return NULL; // Verifica que la lista y el hash no sean NULL
    }

    // Calcula la longitud total necesaria para la concatenación
    size_t total_length = strlen(hash);
    string_proc_node* current_node = list->first;

    while (current_node != NULL) {
        if (current_node->type == type) {
            total_length += strlen(current_node->hash);
        }
        current_node = current_node->next;
    }

    // Reserva memoria para el resultado
    char* result = (char*)malloc(total_length + 1); // +1 para el terminador nulo
    if (result == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para la concatenación.\n");
        return NULL;
    }

    // Copia el prefijo inicial
    strcpy(result, hash);

    // Concatena los hashes de los nodos que coincidan con el tipo
    current_node = list->first;
    while (current_node != NULL) {
        if (current_node->type == type) {
            strcat(result, current_node->hash);
        }
        current_node = current_node->next;
    }

    return result;
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

// void string_proc_node_destroy(string_proc_node* node) {
//     if (node == NULL) {
//         return; // Evita operar sobre un puntero nulo
//     }
//     free(node); // Libera la memoria del nodo
// }


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