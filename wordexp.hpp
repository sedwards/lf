// wordexp.hpp
//
// Class to wrap the C wordexp() function.  Takes a string and expands it,
// doing both environment variable substitution and filename globbing.
// Then splits the string into tokens (delimited by whitespace) and sets
// up an array of C strings, argv(), plus a count of tokens, argc().
//
// This is basically a simple wrapper; it's a class so that the
// destructor will make sure the memory allocated by wordexp() gets
// properly cleaned up.  I doubt if anyone will ever call the copy
// constructor or operator=(), but they are defined so that they will
// work correctly if anyone ever does.
//
// Author: Steve R. Hastings <steve@hastings.org>



// example of how to use this:
//
// wordexp w(s_cmd_line);	// create object and parse cmd line
//
//	if (w.success())
//		// ...do something with w.argc() and w.argv()
//  else
//      // ...handle error from bad cmd line string
//
// w.parse(s_other_cmd_line);	// reuse object to parse another cmd line



#ifndef WORDEXP_HPP

#define WORDEXP_HPP



#include "util.hpp"



// WRDE: wordexp() result code enum

enum WRDE
{
	wrde_success = 0,
	wrde_badchar,
	wrde_badval,
	wrde_cmdsub,
	wrde_nospace,
	wrde_syntax,
	wrde_unknown,
};



typedef char const* const* ARGV;

class wordexp
{
	private:
		void *p;	// private data
		std::string s;	// last string parsed
		WRDE wrde_status;

	public:
		wordexp();
		wordexp(CSZ csz);
		wordexp(CSREF s);
		wordexp(const wordexp& rhs);
		~wordexp();
		wordexp& operator=(const wordexp& rhs);

		// you can explicitly call parse() to parse a different string.
		void parse(CSZ csz="");
		void parse(CSREF s);

		// status() returns a WRDE code with the status of the last parse
		WRDE status() const;
		// success() is true if status() is wrde_success, false otherwise
		bool success() const;

		// str() returns the last string parsed
		CSREF str() const;
		// argc() returns how many arguments were found by the parse
		int argc() const;
		// argv[] is an array of pointers to the arguments found.
		ARGV argv() const;
};



// success()
//
// Returns true if the last parse succeeded, false otherwise.
// You should check this before you trust the argc() and argv() values.
// If you want to know what the actual error was if this was false, you
// can check the status() property.

inline bool
wordexp::success() const
{
	bool b = (status() == wrde_success);

	return b;
}



#endif // WORDEXP_HPP
