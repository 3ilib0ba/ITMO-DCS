/*
 * my_string.c
 *
 *  Created on: Oct 29, 2023
 *      Author: kastr
 */
#include <stddef.h>

#include "my_string.h"

//public functions start

char* concat(char *s1, char *s2) {
	char *result = malloc(strlen(s1) + strlen(s2) + 1);
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}


int parse_number(char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
      if (!isdigit(str[i])) return 0;
    }
    return 1;
}

//public functions end
