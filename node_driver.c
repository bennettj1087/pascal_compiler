#include <stdio.h>
#include "node.h"

int main() {
	int choice;
	char buffer[32];
	node_t *start, *p = NULL;

	start = NULL;
	while(1) {
		fprintf( stderr, "search[0] insert[1] delete[2]? " );
		scanf( "%d", &choice);
		if( choice == 0 ) {
			fprintf(stderr, "Name to search: ");
			scanf( "%s", buffer );
			if( (p = search( start, buffer )) != NULL ) fprintf( stderr, "Found %s!\n", p->name );
			else fprintf( stderr, "%s could not be found.\n", buffer );
		}
		else if( choice == 1) {
			fprintf( stderr, "Name to insert: ");
			scanf("%s", buffer );
			start = insert( start, buffer );
			fprintf( stderr, "Inserted %s.\n", start->name );
		}
		else if( choice == 2) {
			start = delete( start );
			fprintf( stderr, "Deleted all entries.\n" );
		}
	}
}

