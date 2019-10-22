// filetest.hpp
//
// Defines the "filetest" functions.
//
// The filetest functions let you call the C stat() function once and get
// an answer.  They are perfect when you just want to check one thing
// (for example, "is this a directory?").
//
// Author: Steve R. Hastings <steve@hastings.org>



#ifndef FILETEST_HPP

#define FILETEST_HPP



#include "util.hpp"



namespace filetest
{
	enum filetype { ft_dir, ft_file, ft_special_file };

	bool check_type(CSREF basename, CSREF path, filetype& ft);
	bool check_type(CSREF pathname, filetype& ft);
};



#endif // FILETEST_HPP
