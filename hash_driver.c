#include <stdio.h>
#include "node.h"
#include "hash.h"

int main() {
	int choice;
	char buffer[32];
	node_t *p;
	hash_t *top;

	p = NULL;
	top = make_hash();

	while(1) {
		fprintf( stderr, "[0: search current scope] [1: search all scopes] [2: insert] [3: push] [4: pop]? " );
		scanf( "%d", &choice);
		if( choice == 0 ) {
			fprintf(stderr, "Name to search: ");
			scanf( "%s", buffer );
			if( (p = hash_search( top, buffer )) != NULL ) fprintf( stderr, "Found %s!\n", p->name );
			else fprintf( stderr, "%s could not be found.\n", buffer );
		}
		else if( choice == 1 ) {
			fprintf(stderr, "Name to search: ");
			scanf( "%s", buffer );
			if( (p = hash_search_scopes( top, buffer )) != NULL ) fprintf( stderr, "Found %s!\n", p->name );
			else fprintf( stderr, "%s could not be found.\n", buffer );
		}
		else if( choice == 2) {
			fprintf( stderr, "Name to insert: ");
			scanf("%s", buffer );
			p = hash_insert( top, buffer );
			fprintf( stderr, "Inserted %s.\n", p->name );
		}
		else if( choice == 3) {
			top = hash_push( top );
		}
		else if( choice == 4 ) {
			top = hash_pop( top );
		}
	}
}

