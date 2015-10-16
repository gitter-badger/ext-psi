#ifndef _PSI_VALIDATOR_H
#define _PSI_VALIDATOR_H

#include "parser.h"

typedef struct PSI_Validator {
	PSI_DATA_MEMBERS;
	void *dlopened;
} PSI_Validator;

PSI_Validator *PSI_ValidatorInit(PSI_Validator *V, PSI_Parser *P);
int PSI_ValidatorValidate(PSI_Validator *V);
void PSI_ValidatorFree(PSI_Validator **V);
void PSI_ValidatorDtor(PSI_Validator *V);

#endif
