#include <stdio.h>
#include "types.h"

char *type_string( int type ) {
	switch(type) {
		case TYPE_INT:
			return "integer";
		case TYPE_REAL:
			return "real";
		case TYPE_NULL:
			return "null";
		case TYPE_BOOL:
			return "boolean";
		default:
			return "undefined";
	}
}

