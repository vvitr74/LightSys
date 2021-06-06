/*
 * print.h
 *
 *  Created on: 01 апр. 2016 г.
 *      Author: Viacheslav Azarov
 *
 ******************************************************************************
 *  This software developed by engineer Viacheslav Azarov with free tools
 *  from STMicroelectronics on personal wish employer Danil Rogalev and saves
 *  all rights all authors and contributors.
 ******************************************************************************
 */

#ifndef PRINT_H_
#define PRINT_H_

void printstr(void (*putf)(char), char *str);
void printint(void (*putf)(char), int value, short int lenght);

#endif /* PRINT_H_ */
