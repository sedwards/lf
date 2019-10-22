// filetest.cpp -- wrappers around C stat() call
//
// The filetest functions are a very lightweight wrapper around stat().
// They are useful when you just want to test one thing about a file.
//
// Note that filetest.hpp does not include the headers for the C stat()
// call.  There are lots of macros and such in those headers; so those
// headers are included in the .cpp file, but not the header file.
//
// <sys/stat.h> seems to play a bunch of games with macros, and macros
// (being handled by the C preprocessor) don't get contained within a
// namespace.  With g++ (the GCC compiler for C++) on my system, if I
// #include <sys/stat.h> and then #include <string>, I get errors in
// <string>!  Better to contain this evil here.
//
// Author: Steve R. Hastings <steve@hastings.org>



#include "filetest.hpp"


// C includes


namespace c_stat
{
	#include <sys/stat.h>
	// note that macros like S_ISDIR() are not contained in namespace

	typedef struct stat struct_stat;
};

typedef c_stat::struct_stat STATBUF;
typedef STATBUF *PSTATBUF;



// for errno
#include <errno.h>

// for stat()
#include <sys/types.h>
#include <unistd.h>


using namespace std;



// filetest::check_type()
//
// Tests the pathname and figures out what type of file it is.
// Returns false if the file does not exist.
// Returns true and modifies the filetype& argument if file exists.

bool
filetest::check_type(CSREF pathname, filetest::filetype& ft)
{
	STATBUF sb;
	bool b;

	int rc = c_stat::stat(pathname.c_str(), &sb);

	// stat() returns an error if the file is too big!  This is annoying
	// if we are just trying to find out the file type.  So, just ignore
	// an EOVERFLOW error status.
	if (rc != 0 && errno != EOVERFLOW)
		return false;

	b = S_ISDIR(sb.st_mode);
	if (b)
	{
		ft = ft_dir;
		return true;
	}

	b = S_ISREG(sb.st_mode);
	if (b)
	{
		ft = ft_file;
		return true;
	}

	// if it exists but wasn't a dir or normal file, call it a special
	ft = ft_special_file;
	return true;
}



bool
filetest::check_type(CSREF basename, CSREF path, filetest::filetype& ft)
{
	string pathname = MakeFullPathName(basename, path);

	return filetest::check_type(pathname, ft);
}
