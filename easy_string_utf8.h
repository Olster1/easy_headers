/*
A simple header include library like Sean Barret's stb libraries. 

This library makes it easy to handle utf8 encoded c strings (null terminated).

You have to #define EASY_STRING_IMPLEMENTATION before including the file to add the implementation part of it. 

////////////////////////////////////////////////////////////////////

How to use:

////////////////////////////////////////////////////////////////////

easyString_getSizeInBytes_utf8(char *str) - get the size of the string in bytes NOT INCLUDING THE NULL TERMINATOR 
easyString_getStringLength_utf8(char *str) - the number of glyphs in the string NOT INCLUDING THE NULL TERMINATOR. This function probably isn't very helpful. 

////////////////////////////////////////////////////////////////////
Probably the most useful function. See example below, but useful for rendering a utf8 string.  
easyUnicode_utf8_codepoint_To_Utf32_codepoint(char **streamPtr, int advancePtr) - get the next codepoint in the utf8 string, & you can choose to advance your string pointer.  

////////////////////////////////////////////////////////////////////

easyUnicode_utf8StreamToUtf32Stream_allocates(char *string) - turn the whole NULL TERMINATED string from utf8 to utf32 encoding
Use easyString_free_Utf32_string(ptr) to free the memory from the function above when finished

////////////////////////////////////////////////////////////////////
String compare functions:
int easyString_stringsMatch_withCount(char *a, int aLength, char *b, int bLength) - compares strings ignoring whether they're null terminated or not
int easyString_stringsMatch_null_and_count(char *a, char *b, int bLen) - compares a null terminated string to a string with length bLen
int easyString_stringsMatch_nullTerminated(char *a, char *b) - compares two null terminated strings 

////////////////////////////////////////////////////////////////////

Examples:

int main(int argc, char *args[]) {

	char *string = "გთხოვთ";
	int len = easyString_getStringLength_utf8(string);
	printf("%d\n", len);
	
	// Will return 6 glyphs 

	return 0;
 }

////////////////////////////////////////////////////////////////////

 int main(int argc, char *args[]) {

	char *str_utf8 = "გთხოვთ";
	char *at = string; //Take copy since str_utf8 is a read only variable
	
	while(*at) {
		unsigned int utf32Codepoint = easyUnicode_utf8_codepoint_To_Utf32_codepoint(&at, 1);
	
		//Example api below if you wanted to render a utf8 string
		Glyph g = findGlyph(utf32Codepoint);
		renderGlyph(g);
		//
	}
	
	return 0;
 }

 ////////////////////////////////////////////////////////////////////

int main(int argc, char *args[]) {

	int doMatch = easyString_stringsMatch_withCount("გთხოვთ", 6, "გთხო", 4);
	//doMatch will return _false_

	int doMatch = easyString_stringsMatch_null_and_count("გთხო", "გთხოვთ", 4);
	//doMatch will return _true_

	int doMatch = easyString_stringsMatch_nullTerminated("გთხო", "გთხოვთ");
	//doMatch will return _false_
	
	return 0;
 }

*/
#ifndef EASY_STRING_UTF8_H
#define EASY_STRING_UTF8_H


#ifndef EASY_STRING_IMPLEMENTATION
#define EASY_STRING_IMPLEMENTATION 0
#endif

#ifndef EASY_HEADERS_ASSERT
#define EASY_HEADERS_ASSERT(statement) if(!(statement)) { int *i_ptr = 0; *(i_ptr) = 0; }
#endif

#ifndef EASY_HEADERS_ALLOC
#include <stdlib.h>
#define EASY_HEADERS_ALLOC(size) malloc(size)
#endif

#ifndef EASY_HEADERS_FREE
#include <stdlib.h>
#define EASY_HEADERS_FREE(ptr) free(ptr)
#endif

///////////////////////************ Header definitions start here *************////////////////////
int easyUnicode_isContinuationByte(unsigned char byte);
int easyUnicode_isSingleByte(unsigned char byte);
int easyUnicode_isLeadingByte(unsigned char byte);
	
//NOTE: this advances your pointer
unsigned int easyUnicode_utf8_codepoint_To_Utf32_codepoint(char **streamPtr, int advancePtr);

int easyString_getSizeInBytes_utf8(char *string);

int easyString_getStringLength_utf8(char *string);

unsigned int *easyUnicode_utf8StreamToUtf32Stream_allocates(char *stream);

void easyString_free_Utf32_string(char *string);

int easyString_stringsMatch_withCount(char *a, int aLength, char *b, int bLength);
int easyString_stringsMatch_null_and_count(char *a, char *b, int bLen);
int easyString_stringsMatch_nullTerminated(char *a, char *b);


///////////////////////*********** Implementation starts here **************////////////////////

#if EASY_STRING_IMPLEMENTATION

// The leading bytes and the continuation bytes do not share values 
// (continuation bytes start with 10 while single bytes start with 0 and longer lead bytes start with 11)

int easyUnicode_isContinuationByte(unsigned char byte) { //10
	unsigned char continuationMarker = (1 << 1);
	int result = (byte >> 6) == continuationMarker;

	return result;
}

int easyUnicode_isSingleByte(unsigned char byte) { //top bit 0
	unsigned char marker = 0;
	int result = (byte >> 7) == marker;

	return result;	
}

int easyUnicode_isLeadingByte(unsigned char byte) { //top bits 11
	unsigned char marker = (1 << 1 | 1 << 0);
	int result = (byte >> 6) == marker;

	return result;	
}


int easyUnicode_unicodeLength(unsigned char byte) {
	unsigned char bytes2 = (1 << 3 | 1 << 2);
	unsigned char bytes3 = (1 << 3 | 1 << 2 | 1 << 1);
	unsigned char bytes4 = (1 << 3 | 1 << 2 | 1 << 1 | 1 << 0);

	int result = 1;
	unsigned char shiftedByte = byte >> 4;
	if(!easyUnicode_isContinuationByte(byte) && !easyUnicode_isSingleByte(byte)) {
		EASY_HEADERS_ASSERT(easyUnicode_isLeadingByte(byte));
		if(shiftedByte == bytes2) { result = 2; }
		if(shiftedByte == bytes3) { result = 3; }
		if(shiftedByte == bytes4) { result = 4; }
		if(result == 1) EASY_HEADERS_ASSERT(!"invalid path");
	} 

	return result;

}

//NOTE: this advances your pointer
unsigned int easyUnicode_utf8_codepoint_To_Utf32_codepoint(char **streamPtr, int advancePtr) {
	unsigned char *stream = (unsigned char *)(*streamPtr);
	unsigned int result = 0;
	unsigned int sixBitsFull = (1 << 5 | 1 << 4 | 1 << 3 | 1 << 2 | 1 << 1 | 1 << 0);
	unsigned int fiveBitsFull = (1 << 4 | 1 << 3 | 1 << 2 | 1 << 1 | 1 << 0);
	unsigned int fourBitsFull = (1 << 3 | 1 << 2 | 1 << 1 | 1 << 0);

	if(easyUnicode_isContinuationByte(stream[0])) { EASY_HEADERS_ASSERT(!"shouldn't be a continuation byte. Have you advanced pointer correctly?"); }
	int unicodeLen = easyUnicode_unicodeLength(stream[0]);
	if(unicodeLen > 1) {
		EASY_HEADERS_ASSERT(easyUnicode_isLeadingByte(stream[0]));
		//needs to be decoded
		switch(unicodeLen) {
			case 2: {
				// printf("%s\n", "two byte unicode");
				unsigned int firstByte = stream[0];
				unsigned int secondByte = stream[1];
				EASY_HEADERS_ASSERT(easyUnicode_isContinuationByte(secondByte));
				result |= (secondByte & sixBitsFull);
				result |= ((firstByte & sixBitsFull) << 6);

				if(advancePtr) (*streamPtr) += 2;
			} break;
			case 3: {
				// printf("%s\n", "three byte unicode");
				unsigned int firstByte = stream[0];
				unsigned int secondByte = stream[1];
				unsigned int thirdByte = stream[2];
				EASY_HEADERS_ASSERT(easyUnicode_isContinuationByte(secondByte));
				EASY_HEADERS_ASSERT(easyUnicode_isContinuationByte(thirdByte));
				result |= (thirdByte & sixBitsFull);
				result |= ((secondByte & sixBitsFull) << 6);
				result |= ((firstByte & fiveBitsFull) << 12);

				if(advancePtr) (*streamPtr) += 3;
			} break;
			case 4: {
				// printf("%s\n", "four byte unicode");
				unsigned int firstByte = stream[0];
				unsigned int secondByte = stream[1];
				unsigned int thirdByte = stream[2];
				unsigned int fourthByte = stream[3];
				EASY_HEADERS_ASSERT(easyUnicode_isContinuationByte(secondByte));
				EASY_HEADERS_ASSERT(easyUnicode_isContinuationByte(thirdByte));
				EASY_HEADERS_ASSERT(easyUnicode_isContinuationByte(fourthByte));
				result |= (thirdByte & sixBitsFull);
				result |= ((secondByte & sixBitsFull) << 6);
				result |= ((firstByte & sixBitsFull) << 12);
				result |= ((firstByte & fourBitsFull) << 18);

				if(advancePtr) (*streamPtr) += 4;
			} break;
			default: {
				EASY_HEADERS_ASSERT(!"invalid path");
			}
		}
	} else {
		result = stream[0];
		if(advancePtr) { (*streamPtr) += 1; }
	}

	return result;
}


int easyString_getSizeInBytes_utf8(char *string) {
    unsigned int result = 0;
    unsigned char *at = (char *)string;
    while(*at) {
        result++;
        at++;
    }
    return result;
}

int easyString_getStringLength_utf8(char *string) {
    unsigned int result = 0;
    unsigned char *at = (char *)string;
    while(*at) {
        easyUnicode_utf8_codepoint_To_Utf32_codepoint((char **)&at, 1);
        result++;
    }
    return result;
}


//TODO: this would be a good place for simd. 
//NOTE: You have to free your string 

//IMPORTANT: string must be null terminated. 
unsigned int *easyUnicode_utf8StreamToUtf32Stream_allocates(char *stream) {
	unsigned int size = easyString_getStringLength_utf8(stream) + 1; //for null terminator
	unsigned int *result = (unsigned int *)(EASY_HEADERS_ALLOC(size*sizeof(unsigned int)));
	unsigned int *at = result;
	while(*stream) {
		char *a = stream;
		*at = easyUnicode_utf8_codepoint_To_Utf32_codepoint(&stream, 1);
		EASY_HEADERS_ASSERT(stream != a);
		at++;
	}
	result[size - 1] = '\0';
	return result;
}

void easyString_free_Utf32_string(char *string) {
	EASY_HEADERS_FREE(string);
}


int easyString_stringsMatch_withCount(char *a, int aLength, char *b, int bLength) {
    int result = 1;
    
    int indexCount = 0;
    while(indexCount < aLength && indexCount < bLength) {
        indexCount++;
        result &= (*a == *b);
        a++;
        b++;
    }
    result &= (indexCount == bLength && indexCount == aLength);
    
    return result;
} 


int easyString_stringsMatch_null_and_count(char *a, char *b, int bLen) {
    int result = easyString_stringsMatch_withCount(a, easyString_getStringLength_utf8(a), b, bLen);
    return result;
}

int easyString_stringsMatch_nullTerminated(char *a, char *b) {
    int result = easyString_stringsMatch_withCount(a, easyString_getStringLength_utf8(a), b, easyString_getStringLength_utf8(b));
    return result;
}

#endif // END OF IMPLEMENTATION
#endif // END OF HEADER INCLUDE


/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/

