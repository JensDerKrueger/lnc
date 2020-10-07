#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
	const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	void * return_adr[1000];
	size_t return_adr_idx = 0;
	
	char* reverse_result;
	char* reverse_inStr;
	
	unsigned long strToInt_result;
	char* strToInt_inStr;
	unsigned char strToInt_inBase;
	
	unsigned char find_result;
	char find_c;
	const char* find_str;
	
	char* convert_inStr = argv[1];
	

	strToInt_inStr = argv[2];
	strToInt_inBase = 10;
	return_adr[return_adr_idx++] = &&MAIN_RETURN1;
	goto STRTOINT; 
MAIN_RETURN1:;	
	unsigned char convert_inBase = strToInt_result;
	
	
	strToInt_inStr = argv[3];
	strToInt_inBase = 10;
	return_adr[return_adr_idx++] = &&MAIN_RETURN2;
	goto STRTOINT; 
MAIN_RETURN2:;	
	
	unsigned char convert_outBase = strToInt_result;
	char * convert_result;	
	return_adr[return_adr_idx++] = &&MAIN_RETURN3;
	goto CONVERT; 
MAIN_RETURN3:;	
	printf("%s -> %s \n", argv[1], convert_result);	
	free(convert_result);
	goto MAIN_END;
	
CONVERT:;

	strToInt_inStr = convert_inStr;
	strToInt_inBase = convert_inBase;
	return_adr[return_adr_idx++] = &&CONVERT_RETURN1;
	goto STRTOINT; 
CONVERT_RETURN1:;	

	unsigned long intToStr_number = strToInt_result;
	unsigned char intToStr_outBase = convert_outBase;
	char* intToStr_result;
	return_adr[return_adr_idx++] = &&CONVERT_RETURN2;
	goto INTTOSTR; 
CONVERT_RETURN2:;
	convert_result = intToStr_result;
	goto *return_adr[--return_adr_idx];
	
INTTOSTR:;	
	// figure out the length of the target string
	int _number = intToStr_number;
	size_t l = 0;
	
INTTOSTR_LOOP_1:;
	unsigned char digit = _number % intToStr_outBase;
	l++;
	_number = _number / intToStr_outBase;
	if (_number > 0) goto INTTOSTR_LOOP_1;

	// do the actual conversion
	char* revOutStr = malloc( (l + 1) * sizeof(char) );	
	size_t pos = 0;
	
INTTOSTR_LOOP_2:;
	digit = intToStr_number % intToStr_outBase;
	revOutStr[pos++] = digits[digit];
	intToStr_number = intToStr_number / intToStr_outBase;
	if (intToStr_number > 0) goto INTTOSTR_LOOP_2;

	revOutStr[pos] = 0;
	
	reverse_inStr = revOutStr;	
	return_adr[return_adr_idx++] = &&INTTOSTR_RETURN1;
	goto REVERSE; 
INTTOSTR_RETURN1:;
	
	char* outStr = reverse_result;
	free(revOutStr);	
	intToStr_result = outStr;
	goto *return_adr[--return_adr_idx];
	
STRTOINT:;	
	unsigned long number = 0;
	unsigned long multi = 1;
	
	reverse_inStr = strToInt_inStr;	
	return_adr[return_adr_idx++] = &&STRTOINT_RETURN1;
	goto REVERSE; 
STRTOINT_RETURN1:;
	
	char* revInStr = reverse_result;
	
	size_t index = 0;
STRTOINT_LOOP_1:;
	if (! (index < strlen(revInStr))) goto STRTOINT_LOOP_1_EXIT;
	
	find_c = revInStr[index];
	find_str = digits;	
	return_adr[return_adr_idx++] = &&STRTOINT_RETURN2;
	goto FIND; 
STRTOINT_RETURN2:;
	
	
	unsigned char strToInt_digit = find_result;
	number += strToInt_digit * multi;
	multi *= strToInt_inBase;
	++index;
	goto STRTOINT_LOOP_1;
STRTOINT_LOOP_1_EXIT:;

	free(revInStr);	
	strToInt_result = number;	
	goto *return_adr[--return_adr_idx];
	
FIND:;
	size_t find_index = 0;
FIND_LOOP_1:;
	if (!(find_index < strlen(find_str))) goto FIND_LOOP_1_EXIT;
	if (!(find_c == find_str[find_index])) goto NO_IF_FIND;
	find_result = find_index;	
	goto *return_adr[--return_adr_idx];
NO_IF_FIND:
	++find_index;
	goto FIND_LOOP_1;
FIND_LOOP_1_EXIT:;
	find_result = 255;	
	goto *return_adr[--return_adr_idx];
	
	
REVERSE:;
	const size_t reverse_l = strlen(reverse_inStr);
	char* result = malloc( (reverse_l + 1) * sizeof(char) );
	
	size_t reverse_index = 0;
REVERSE_LOOP_1:;
	if (! (reverse_index<reverse_l)) goto REVERSE_LOOP_1_EXIT;
	result[reverse_index] = reverse_inStr[(reverse_l-1)-reverse_index];
	++reverse_index;
	goto REVERSE_LOOP_1;
REVERSE_LOOP_1_EXIT:;	
	result[reverse_l] = 0;
	reverse_result = result;
	goto *return_adr[--return_adr_idx];

MAIN_END:;
	return EXIT_SUCCESS;
}