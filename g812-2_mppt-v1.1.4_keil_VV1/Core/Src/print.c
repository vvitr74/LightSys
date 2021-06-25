
/*
 * print.c
 *
 *  Created on: 1. 04. 2016
 *      Author: Viacheslav Azarov
 *
 ******************************************************************************
 *  This software developed by engineer Viacheslav Azarov with free tools
 *  from STMicroelectronics on personal wish employer Danil Rogalev and saves
 *  all rights all authors and contributors.
 ******************************************************************************
 */
#include <stdlib.h>
#include "print.h"

void printstr(void (*putf)(char), char * str)
{
	while (*str) (*putf)(*str++);
}

void printint(void (*putf)(char), int value, short int lenght)
{
	div_t res;

	char buf[12];
	char * pos;

// Initialize
	if (lenght > 11) lenght = 11;
	res.quot = abs(value);
	*(pos = &buf[11])='\0';

// Calculate numbers
	do
	 {
	   res = div(res.quot,10);
	   *(--pos) = '0'+ res.rem;
	   lenght--;
	 } 
	while (res.quot > 0);

// Add sign
	if (value < 0) {*(--pos) = '-'; lenght--;}
	
// Fill spaces
	while (lenght-- > 0) *(--pos) = ' ';

	printstr(putf, pos);
}
