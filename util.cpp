// util.cpp
//
// Generally useful utility functions.
//
// Author: Steve R. Hastings <steve@hastings.org>



#include <cctype>
#include <cstdlib>


#include "util.hpp"



namespace	// begin unnamed namespace
{


// ValidateAtoI()
//
// Surprisingly, the standard C and C++ library functions for number
// conversion do not return a useful error status.  Any conversion error
// results in a zero value returned, which means that if zero is an
// acceptable input value, you cannot tell whether the input was valid
// or not; which in turn means you cannot give a good error message back
// to the user if the input is garbled.
//
// The simplest solution is to validate a string, and then if it's
// valid, run atoi() on that string.  Here's the validator.
//
// This will accept white space before and after the number, and a
// single unary + or - sign.

inline bool
ValidateAtoI(CSZ sz)
{
	bool sawdigit = false;
	CSZ p = sz;

	// skip any leading whitespace
	while (*p != '\0')
	{
		if (!isspace(*p))
			break;

		++p;
	}

	// accept unary + or -
	if (*p == '-' || *p == '+')
		++p;

	// check digits
	while (*p != '\0')
	{
		if (!isdigit(*p))
			break;

		sawdigit = true;	
		++p;
	}

	// skip any trailing whitespace
	while (*p != '\0')
	{
		if (!isspace(*p))
			break;

		++p;
	}

	// if we reached end of string, and we saw at least one digit, it's
	// a good number for conversion
	if (*p == '\0' && sawdigit)
		return true;

	return false;
}


} // end unnamed namespace



// ConvertAtoI()
//
// Returns true on successful conversion.
// On success, converted value is stored to reference i.
//
// Calls ValidateAtoI() to make sure the input is good, then relies on
// atoi() to do the conversion.  This is portable anywhere atoi() is
// correct, where my hand-conversion code might not be.

bool
ConvertAtoI(CSZ sz, int& i)
{
	bool b = ValidateAtoI(sz);

	if (!b)
		return false;

	i = atoi(sz);
	return true;
}


bool
ConvertAtoI(CSREF s, int& i)
{
	return ConvertAtoI(s.c_str(), i);
}
