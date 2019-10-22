// util.hpp
//
// General-purpose stuff, useful in almost any source file.
//
// Author: Steve R. Hastings <steve@hastings.org>



#ifndef UTIL_HPP

#define UTIL_HPP


#include <string>



typedef const char *CSZ;	// may not assign through this pointer

typedef const std::string& CSREF;	// const string reference

inline CSZ begin(CSREF s) { return s.data(); }
inline CSZ end(CSREF s) { return s.data() + s.length(); }



// nop()
//
// Debugging stub.  Just put a call to nop() any place you want to be
// able to quickly and easily set a breakpoint; set a breakpoint on
// "nop()" and run.

inline void
nop()
{
}



#ifdef DEBUG


#include <assert.h>

#define Assert(b)	assert(b)


#else // not DEBUG


inline void
Assert(bool b)
{
}


#endif // DEBUG



bool ConvertAtoI(CSZ sz, int& i);
bool ConvertAtoI(CSREF s, int& i);



// IsNum()
//
// Returns true if the string contains only digits.
// A zero-length string will return true as well.

inline bool
IsNum(CSREF s)
{
	CSZ b = begin(s);
	CSZ e = end(s);
	for (CSZ p = b; p < e; ++p)
	{
		if (!isdigit(*p))
			return false;
	}

	return true;	// only digits found
}



const char dir_sep_char = '/';



// IsAbsolutePath()
//
// Returns true if the pathname is an absolute path, false if relative.

inline bool
IsAbsolutePath(CSZ pathname)
{
	bool b = (pathname[0] == dir_sep_char);

#ifdef WINDOWS
	if (pathname[0] == ch_backslash && pathname[1] == ch_backslash)
		b = true;	// example: \\server\share\path\to\file.txt
	if (pathname[1] == ch_colon)
		b = true;	// example: C:\path\to\file.txt
#endif // WINDOWS

	return b;
}


inline bool
IsAbsolutePath(CSREF pathname)
{
	return IsAbsolutePath(pathname.c_str());
}



// MakeFullPathName()
//
// Given a file name (basename) and a path, joins them together into
// a path name.  Returns the new pathname.

inline std::string
MakeFullPathName(CSREF basename, CSREF path)
{
	std::string s = path;
	int len = s.length();

	if (len > 0)
	{
		if (s[len - 1] != dir_sep_char)
			s += dir_sep_char;
	}

	s.append(basename);

	return s;
}



// InitialMatch()
//
// Returns true if the initial characters of s1 match all characters in s0.
// s1 may be the same length as s0, or longer.

inline bool
InitialMatch(CSZ s0, CSZ s1)
{
	if (s0 == NULL || s1 == NULL)
		return false;

	for (;;)
	{
		if (*s0 == '\0')
			return true;

		if (*s0++ != *s1++)
			return false;

		// Note that if s1 is shorter than s0, this will catch it
		// because a '\0' won't match the character in s0.
	}

	Assert(false);
	return false;	// impossible to reach here
}



inline bool
InitialMatch(CSREF s0, CSREF s1)
{
	return InitialMatch(s0.c_str(), s1.c_str());
}



// GetCwd()
//
// Returns the current working directory as a string.

inline std::string
GetCwd()
{
	char cwd_buf[512];

	getcwd(cwd_buf, sizeof(cwd_buf));

	return std::string(cwd_buf);
}



#endif // UTIL_HPP
