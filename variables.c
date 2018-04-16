#include "variables.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
const char *strduplicate(const char *s);
struct variableTable *getVariableByKey( const char* key);
/*
 * representation of element in variable table.
 */
struct variableTable {
    const char *key;
    const char *value;
};

int actualTableSize = 0;
//array of the table.
struct variableTable *table[100];
const char* lookup_variable( const char* key )
{
	struct variableTable *temp = getVariableByKey(key);
    if(temp) {
        return strdup(temp->value);
    }
    return getenv(key);
}

void set_variable(const char* key , const char* value )
{
	struct variableTable *element;
    if (!(element = getVariableByKey(key))) {
        element = (struct variableTable *) malloc(sizeof(*element));
        if (!element || !(element->key = strdup(key))) {
            printf("can't store a null variable or a null key");
            return;
        }
        if (!element || !(element->value = strdup(value))) {
            printf("can't store a null variable or a null value");
            return;
        }
        char *envElement;
        if (!(envElement = getenv(key))) {
            table[actualTableSize] = (struct variableTable *) malloc(sizeof(*element));
            table[actualTableSize]->key = strdup(element->key);
            table[actualTableSize]->value = strdup(element->value);
            actualTableSize++;
        } else {
            table[actualTableSize] = (struct variableTable *) malloc(sizeof(*element));
            table[actualTableSize]->key = strdup(element->key);
            table[actualTableSize]->value = strdup(element->value);
            actualTableSize++;
        }
    } else {
        element = getVariableByKey(key);
        element->value = strdup(value);
    }
}

void print_all_variables( void )
{
	int i = 0;
    while(i < actualTableSize) {
        printf("%s=%s",table[i]->key, table[i]->value);
        i++;
    }
}

/*
 * function to get the variable needed in table given its key.
 */
struct variableTable *getVariableByKey( const char* key) {
    int i;
    for (i = 0; i < actualTableSize; i++) {
        if (strcmp(table[i]->key , key) == 0 ) {
            return table[i];
        }
    }
    return NULL;
}
