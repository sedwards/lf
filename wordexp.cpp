// wordexp.cpp
//
// Class to wrap the C wordexp() function.  Takes a string and expands it,
// doing both environment variable substitution and filename globbing.
// Then splits the string into tokens (delimited by whitespace) and sets
// up an array of C strings, argv(), plus a count of tokens, argc().
//
// See the header file for example code of how to call this.
//
// Author: Steve R. Hastings <steve@hastings.org>



#include "wordexp.hpp"

using namespace std;


const string s_empty("");



// wrap C header in a namespace
// Without this, the name "wordexp" would not be available for the class
// name (this header defines wordexp() as a function).

namespace c_wordexp
{

#include <wordexp.h>

}; // end namespace c_wordexp



typedef c_wordexp::wordexp_t WXT;
typedef WXT *PWXT;



namespace
{


// WrdeFromRc()
//
// Given the integer result code from the C wordexp() call, converts to
// the WRDE enumerated type defined in the header file.

WRDE
WrdeFromRc(int rc)
{
	switch (rc)
	{
	case 0:
		return wrde_success;
	case c_wordexp::WRDE_BADCHAR:
		return wrde_badchar;
	case c_wordexp::WRDE_BADVAL:
		return wrde_badval;
	case c_wordexp::WRDE_CMDSUB:
		return wrde_cmdsub;
	case c_wordexp::WRDE_NOSPACE:
		return wrde_nospace;
	case c_wordexp::WRDE_SYNTAX:
		return wrde_syntax;
	}

	return wrde_unknown;
}

} // end unnamed namespace



// constructors
// 
// The opaque pointer in the class stores the WXT structure, used by the
// call to wordexp().  The first call to wordexp() will set this up;
// later calls can reuse it with a reuse flag.
//
// If the pointer is NULL when parse() is called, it will allocate the
// WXT structure, so the constructors can leave that job for parse() to
// do.  And parse() can check the pointer to see if it's null, as a test
// to see whether or not to pass in the reuse flag to the call to
// wordexp().

wordexp::wordexp()
{
	p = 0;
	wrde_status = wrde_unknown;
}



wordexp::wordexp(CSZ csz)
{
	p = 0;
	wordexp::parse(csz);
}



wordexp::wordexp(CSREF s)
{
	p = 0;
	wordexp::parse(s);
}



wordexp::wordexp(const wordexp& rhs)
{
	p = 0;
	wordexp::parse(rhs.str());
}



wordexp& wordexp::operator=(const wordexp& rhs)
{
	// Since a parse() is fairly expensive, it's worth a few cycles to see if
	// we can avoid doing the work.

	if (this == &rhs)	// self-assignment means no work needed
		return *this;

	if (s.compare(rhs.str()) == 0)	// if lhs already parsed the same string
		return *this;		// no work needed

	wordexp::parse(rhs.str());

	return *this;
}



// destructor

wordexp::~wordexp()
{
	if (p == 0)
		return;

	PWXT pwxt = static_cast<PWXT>(p);

	wordfree(pwxt);

	delete pwxt;

	p = 0;
}



// parse()
//
// Does the actual work of calling the C wordexp().

void
wordexp::parse(CSREF s_parse)
{
	int flags = c_wordexp::WRDE_NOCMD;

	s = s_parse;

	if (p == 0)		// p doubles as a flag: is this the first call?
		p = new WXT;	// first call, so we must allocate struct
	else
		flags |= c_wordexp::WRDE_REUSE;	// not first call; need REUSE flag

	PWXT pwxt = static_cast<PWXT>(p);
	

	int rc = c_wordexp::wordexp(s.c_str(), pwxt, 0);

	wrde_status = WrdeFromRc(rc);
}



void
wordexp::parse(CSZ csz)
{
	string s;

	if (csz == NULL)
		s = s_empty;
	else
		s = string(csz);

	wordexp::parse(s);
}



// properties
//
// These properties are just for inspecting the current state of the
// object.



// status()
//
// Returns the status code from the last parse, as a WRDE code.

WRDE
wordexp::status() const
{
	if (p == 0)
		return wrde_unknown;

	return wrde_status;
}



// str()
//
// Returns the last string parsed.

CSREF
wordexp::str() const
{
	if (p == 0)
		return s_empty;

	return s;
}



// argc() and argv()
//
// Returns the argc/argv results of the last parse.  May not be valid if the
// last parse failed; in particular, if the last parse failed argv() is
// probably a null pointer.
//
// To determine whether the last parse failed or not you can simply
// check the success() property.  If you actually care what the error
// was, you can of course check status().

int
wordexp::argc() const
{
	if (p == 0)
		return 0;

	PWXT pwxt = static_cast<PWXT>(p);

	return pwxt->we_wordc;
}



ARGV
wordexp::argv() const
{
	if (p == 0)
		return 0;

	PWXT pwxt = static_cast<PWXT>(p);

	return pwxt->we_wordv;
}
