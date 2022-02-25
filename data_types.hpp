#ifndef DATA_TYPES_HPP
#define DATA_TYPES_HPP

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <bitset>
#include <map>
#include <cmath>
#include <ctgmath>
#include "subatomic.hpp"

/*
Accepts a 16-bit unsigned int and returns each individual byte (LSB in index 0).
Populates 'var_bytes', without clearing.

Returns false if data cannot fit in specified size.
*/
bool uint16_bytes(int x, vector<int>& var_bytes){

	uint32_t full_value = x;
	uint32_t mask8 = 255;
	var_bytes.push_back(full_value & mask8);
	var_bytes.push_back(full_value & (mask8 << 8));

	// Check bounds
	return (x <= 65535 && x >= 0);
}

/*
Accepts an 8-bit signed int and returns each individual byte (LSB in index 0).
Populates 'var_bytes', without clearing.

Represents as a twos complement number.

Returns false if data cannot fit in specified size.
*/
bool int8_bytes(int x, vector<int>& var_bytes){

	// Explaination: https://stackoverflow.com/questions/25754082/how-to-take-twos-complement-of-a-byte-in-c
	uint8_t full_value = x;
	var_bytes.push_back(-full_value);

	// Check bounds
	return (x <= 127 && x >= -128);
}

/*
Accepts an 8-bit signed int and returns each individual byte (LSB in index 0).
Populates 'var_bytes', without clearing.

Represents as a twos complement number.

Returns false if data cannot fit in specified size.
*/
bool int32_bytes(int x, vector<int>& var_bytes){

	// Explaination: https://stackoverflow.com/questions/25754082/how-to-take-twos-complement-of-a-byte-in-c
	uint32_t full_value = x;
	uint32_t twos_comp = -full_value;

	uint32_t mask = 255;
	var_bytes.push_back(twos_comp & mask);
	var_bytes.push_back(twos_comp & (mask << 8));
	var_bytes.push_back(twos_comp & (mask << 16));
	var_bytes.push_back(twos_comp & (mask << 24));

	// Check bounds
	return (x <= 2147483647 && x >= -2147483648);
}

#endif
