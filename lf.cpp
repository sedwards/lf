// lf.cpp -- "list files" program
// 
// Lists files in a directory.  Subdirectories at top, then files in a
// terse format, sorted by extensions, like this:
//
// DIRS: bak junk
//     : apples filelist.md5sum makefile README
//    c: bar baz foo
//    o: baz foo
//  txt: readme
//
// Files with no extension are listed under the directories, followed by
// the various extensions, in alpha sort order.  Files are sorted.  By
// default, the sort order is the user's default locale sort.  If an
// extension is very long (default: longer than 4 characters), the file
// will be treated as if it did not have an extension, as with
// filelist.md5sum in the example.
//
// Removing the extensions compresses the listing a bit, and you can fit
// quite a bit of information into one screen.  Also, it can be valuable
// to see your files sorted by extension; it makes it easier to
// understand what kind of files are in the directory.
//
// Author: Steve R. Hastings <steve@hastings.org>



// C includes
#include <dirent.h>


// C++ includes
#include <iostream>
#include <list>
#include <locale>
#include <map>
#include <set>
#include <string>

using namespace std;

#include "filetest.hpp"
using namespace filetest;

#include "wordexp.hpp"

#include "lf.hpp"



const string s_empty("");
const string s_dot(".");
const string s_space(" ");

const char ch_dash = '-';
const char ch_dot = '.';
const char ch_eq = '=';
const char ch_nul = '\0';
const char ch_space = ' ';



// lf_options -- bundle of options to control the way lf works
//
// Tidier than a handful of globals.

namespace {
	enum sort_method { sort_locale, sort_ascii, sort_ascii_ic };
};

class lf_options
{
	public:
		// name of program as invoked (a copy of argv[0])
		string progname;

		// a copy of the LFOPTS env var
		string lfopts;

		// sort filenames using locale collate order, ASCII order, or
		// case-insensitive ASCII order
		sort_method sort;

		// if true, filenames will be forced to lower-case
		bool force_lower;

		// if true, hidden filenames will be shown
		bool show_all;

		// if false, only print a directory argument; if true, slurp it
		bool slurp_dir_arg;

		// how much additional verbosity should be shown?
		int verbose_level;

		// format lines for what maximum width?
		int line_width;
		// try to leave blank this many spaces on the right of each line
		int line_margin;
		// computed line width (width after reserving margin space)
		int width() { return line_width - line_margin; };

		// use this width for displaying extensions
		int ext_width;
		// longest accepted extension; longer extensions will be ignored
		int ext_limit;

		// string to separate filenames in listing
		string name_separator;
		// string to separate extensions from the filenames
		string ext_separator;		// no option to change this

		// string to use to replace spaces in a file name
		bool replace_spaces;
		string s_replace_space;

		lf_options();
};



lf_options::lf_options()
{
	// set progname default value; it will be changed later to argv[0]
	progname = LF_CANONICAL_NAME;

	sort = sort_locale;
	force_lower = false;
	show_all = false;
	slurp_dir_arg = true;
	verbose_level = 1;
	line_width = 80;
	line_margin = 0;
	ext_width = 4;
	ext_limit = 4;
	name_separator = s_space;
	ext_separator = ": ";

	replace_spaces = false;
	s_replace_space = s_space;
}

lf_options options;



// PrintStrings()
//
// Prints multi-string messages.

void
PrintStrings(ostream& o, CSZ const strings[], int strings_max, int exit_status)
{
	for (int i = 0; i < strings_max; ++i)
		o << strings[i] << endl;

	exit(exit_status);
}



// Err()
//
// Prints an error message.  Always puts the program name first.

void
Err(CSREF s0, CSREF s1 = "")
{
	const string s_separator(": ");
	string s;

	s = options.progname;

	s.append(s_separator);
	s.append(s0);

	if (s1.length() > 0)
	{
		s.append(s_separator);
		s.append(s1);
	}

	cerr << s << endl;
}



// ErrExit()
//
// Prints an error message and exits.

void
ErrExit(CSREF s0, CSREF s1 = "")
{
	Err(s0, s1);

	exit(2);
}



// global locale variables
// These enable locale-specific collation and lower-case.

locale def_locale;
const ctype<char> *pdef_ctype;
const collate<char> *pdef_collate;



// InitLocale()
//
// This should be called after argument processing.  If there is
// something wrong with the user's setup and the locale initialization
// fails, lf prints a warning message; but the user can specify the
// --ascii or --ascii-ic options explicitly to shut up the message.

void
InitLocale()
{
	try
	{
		// empty string means: get user's preferred locale
		def_locale = locale("");
		const ctype<char>& ctype_ref =
				use_facet<ctype<char> >(def_locale);
		const collate<char>& collate_ref =
				use_facet<collate<char> >(def_locale);

		pdef_ctype = &ctype_ref;
		pdef_collate = &collate_ref;
	}
	catch (...)
	{
		// could not get user's locale, so fall back to "C" locale
		if (options.sort == sort_locale)	// User wants locale...
			Err(err_str[bad_locale]);	// ...but we can't do it; so warn.

		def_locale = locale("C");	// should never fail
		const ctype<char>& ctype_ref =
				use_facet<ctype<char> >(def_locale);
		const collate<char>& collate_ref =
				use_facet<collate<char> >(def_locale);

		pdef_ctype = &ctype_ref;
		pdef_collate = &collate_ref;
	}
}



// mycompare
//
// Custom comparison function that can be used to collate files in
// ASCII, ASCII case-insensitive, or user's locale order.
//
// Used in the specification of the sorted data structures (see below).

struct mycompare
{
	bool
	operator()(CSREF s0, CSREF s1) const
	{
		int rc;

		if (options.sort == sort_ascii)
			rc = strcmp(s0.c_str(), s1.c_str());
		else if (options.sort == sort_ascii_ic)
			rc = strcasecmp(s0.c_str(), s1.c_str());
		else
		{
			Assert(options.sort == sort_locale);
			rc = (*pdef_collate).compare(begin(s0), end(s0),
					begin(s1), end(s1));
		}

		return rc < 0;
	}
};



// SetStrings()
//
// Set up all the strings for the user's language.  If we don't have
// any strings that match the user's language or cannot determine the
// user's language, default to English strings.

void
SetStrings()
{
	CSZ lang = getenv("LANG");

	if (lang == NULL)
	{
		// cannot determine user's language so default to English
		lang_en::SetStrings();
		return;
	}

	if (InitialMatch("fr", lang))
		lang_fr::SetStrings();
	else
		lang_en::SetStrings();	// default to English
}



#ifdef DEBUG
// CheckStrings()
//
// It's easy to make a mistake and end up with a string not translated.
// This is a simple check to make sure that SetStrings() has set up
// every single string.  Of course this cannot check the correctness
// of the strings, but it at least makes sure that they are all there.
//
// This will be especially important if a new version of lf adds
// new strings, and there are many localized versions.

void
CheckStrings()
{
	CSZ const bad = "bad";

	for (int i = 0; i < ARG_STRINGS_MAX; ++i)
		arg_str[i] = bad;
	for (int i = 0; i < VERBOSE_STRINGS_MAX; ++i)
		verbose_str[i] = bad;
	for (int i = 0; i < ERR_STRINGS_MAX; ++i)
		err_str[i] = bad;

	usage_strings = 0;
	usage_strings_max = 0;
	version_strings = 0;
	version_strings_max = 0;
	license_strings = 0;
	license_strings_max = 0;

	dirs_str = bad;
	SetStrings();

	for (int i = 0; i < ARG_STRINGS_MAX; ++i)
		if (arg_str[i] == bad)
			Assert(false);
	for (int i = 0; i < VERBOSE_STRINGS_MAX; ++i)
		if (verbose_str[i] == bad)
			Assert(false);
	for (int i = 0; i < ERR_STRINGS_MAX; ++i)
		if (err_str[i] == bad)
			Assert(false);

	if (usage_strings == 0)
			Assert(false);
	if (usage_strings_max == 0)
			Assert(false);
	if (version_strings == 0)
			Assert(false);
	if (version_strings_max == 0)
			Assert(false);
	if (license_strings == 0)
			Assert(false);
	if (license_strings_max == 0)
			Assert(false);

	if (dirs_str == bad)
			Assert(false);
}
#endif



// core data structures
//
// The true heart of this program is this group of data structures:
//
// args_try_list
// 		This is a list of arguments to try processing as files or
// 		directories.  If you invoke "lf *.c *.h" then all the C and
// 		header file names will be saved in here.  We want to check all
// 		arguments before processing any file names, so examples like
// 		"lf *.c *.h -A" will work as the user wanted.
//
// ext_set
//		This is where we keep track of all extensions seen so far.  When
//		the file listing is finally printed, we just iterate through
//		this to find all the extensions.  Because sets are sorted, this
//		means that the listing will have the extensions sorted.  (The
//		ext_map data structure does not keep the extensions in any kind
//		of sorted order.  This does, so it's convenient to have it.)
//
// dirs_set
// 		This is where we keep track of directories seen.  When printing
// 		the file listing, we iterate through this to print the "DIRS:"
// 		line.
//
// ext_map
//		A mapping from extensions onto basenames.  For each extension
//		there is a set of basenames.

typedef list<string> LIST_STRING;
typedef LIST_STRING *PLIST_STRING;
LIST_STRING args_try_list;

typedef set<string, mycompare> SET_STRING;
typedef SET_STRING *PSET_STRING;
SET_STRING ext_set;
SET_STRING dirs_set;

typedef map<string, PSET_STRING> MAP_STRING_SET_STRING;

MAP_STRING_SET_STRING ext_map;



// AddStringToSet()
//
// Looks up a string in a set; if the string does not exist in the set
// already, adds that string.  Returns true if string was added.

bool
AddStringToSet(SET_STRING& set_string, CSREF s)
{
	SET_STRING::const_iterator l;

	l = set_string.find(s);
	bool added = (l == set_string.end());
	if (added)
	{
		// string not in set so insert it
		pair<SET_STRING::const_iterator, bool> insert_result;
		insert_result = set_string.insert(s);
	}

	return added;
}



// UpdateMap()
//
// Given a file name and the file's extension, adds that file name to
// ext_map.  Also makes sure the extension is in the ext_set.

void
UpdateMap(CSREF basename, CSREF ext)
{
	SET_STRING::const_iterator l;

	bool added = AddStringToSet(ext_set, ext);
	if (added)
	{
		// extension never seen before, so make a new list of names
		PSET_STRING p = new SET_STRING;
		ext_map[ext] = p;
	}

	pair<SET_STRING::const_iterator, bool> insert_result;
	insert_result = ext_map[ext]->insert(basename);
}



// ForceToLower()
//
// Forces a string to lower-case, using the user's locale tolower().

void
ForceToLower(string& s)
{
	int len = s.length();
	char buf[len + 1];

	s.copy(buf, len);
	buf[len] = ch_nul;

	(*pdef_ctype).tolower(buf, buf + len + 1);

	s = string(buf);
}



// ReplaceSpaces()
//
// Replaces any spaces in a string with options.s_replace_space.

void
ReplaceSpaces(string& s)
{
	const int space_len = 1;	// length of a space
	const int rlen = options.s_replace_space.length();

	int i = 0;

	for (;;)
	{
		i = s.find(ch_space, i);
		if (i < 0)
			return;
		s.replace(i, space_len, options.s_replace_space, 0, rlen);
		i += rlen;
	}
}



// TransformName()
//
// Applies any transformations specified by user's options to a file name.

void
TransformName(string& name)
{
	if (options.force_lower)
		ForceToLower(name);

	if (options.replace_spaces)
		ReplaceSpaces(name);
}



// AddDir()
//
// Adds a name to the dirs_set.

void
AddDir(CSREF dir_name)
{
	string name = dir_name;

	TransformName(name);

	pair<SET_STRING::const_iterator, bool> insert_result;
	insert_result = dirs_set.insert(name);
}



// ScanForExtension()
//
// Scans backwards through a file name, looking for an extension.
// Applies several rules to decide whether to keep an extension it
// finds.  If it doesn't find one, or if the extension it finds doesn't
// qualify, returns -1; otherwise, returns the index into the string at
// which the extension starts.

int
ScanForExtension(CSREF name)
{
	int i_dot = name.rfind(ch_dot);	// index to the last dot in file name
	int i_ext = i_dot + 1;	// one past the dot should be the extension

	// i_dot will be < 0 if no dot was found; a no-ext file.
	// i_dot will be == 0 if the first char of name is a dot.
	// Initial dot is for a hidden file, not for a filename that is all
	// extension; so if i_dot is 0, call that a no-ext file.
	// Even under MS Windows where an initial dot isn't special, this is
	// the right thing to do; a null filename would not make sense.

	if (i_dot <= 0)
		return -1;

	int ext_len = name.length() - i_ext;
	if (ext_len > options.ext_limit)
		return -1;	// extension is too long!

	// *NIX has lots of files of the form "foo-2.18".  We want to treat
	// this as a file with no extension, not as a ".18" file with a
	// basename of "foo-2".  The following code detects this.

	// is the extension numeric?
	if (!IsNum(name.substr(i_ext)))
		return i_ext;	// not numeric, so it's fine; return it

	string temp = name.substr(0, i_dot);
	int i = temp.rfind(ch_dash);
	if (i < 0)
		return i_ext;	// no '-' found, so extension is good!

	++i; 	// advance past '-'
	if (!IsNum(temp.substr(i)))
		return i_ext;	// non-numeric found after '-' so it's good

	return -1;	// looks like "-2.18" so call it no ext
}



// AddToMap()
//
// Adds a file name to the ext_map data structure.

void
AddToMap(CSREF name)
{
	string basename, ext;

	int i = ScanForExtension(name);

	if (i <= 0)
		basename = name;	// no extension; basename is whole name
	else
		basename = name.substr(0, i - 1);	// name not including dot

	if (i <= 0)
		ext = s_empty;
	else if (i == name.length())
		ext = s_dot;	// '.' was last char of name; use "." as ext
	else
		ext = name.substr(i, name.length());

	TransformName(basename);
	TransformName(ext);

	UpdateMap(basename, ext);
}



// SlurpTryName()
//
// This checks a file name to see if it is a directory or a file,
// and does the right thing with it.  For a directory name, that
// just means calling AddDir(), because lf does not have an option to
// recursively slurp directories.
//
// This is always called by SlurpDir() so there is always a "path" from
// which the files are being slurped.
//
// FUTURE: when lf gets modified to work in a MS Windows environment,
// the check for hidden files will need to be modified.  It will need
// to call the Windows equivalent of stat(), to check if a file has a
// "hidden" attribute.

void
SlurpTryName(CSREF name, CSREF path, bool keep_path)
{
	if (options.show_all == false && name[0] == ch_dot)
		return;	// skip files starting with dot

	string pathname = MakeFullPathName(name, path);

	filetype ft;
	bool b = check_type(pathname, ft);
	if (!b)
	{
		Err(name, err_str[bad_filename]);
		return;
	}

	if (ft == ft_dir)
	{
		if (keep_path)
			AddDir(pathname);
		else
			AddDir(name);
	}
	else
	{
		if (keep_path)
			AddToMap(pathname);
		else
			AddToMap(name);
	}
}



// SlurpDir()
//
// Opens a directory, lists out all the files and directories in it, and
// saves all the names.  If keep_path is true, all the files will have
// the path saved as part of the name: files in directory foo will be
// saved as foo/filename.
//
// FUTURE: when lf gets modified to work in a MS Windows environment, an
// alternate version of this function will be needed under #ifdef.

void
SlurpDir(CSREF path, bool keep_path)
{
	DIR *pdir;

	pdir = opendir(path.c_str());

	struct dirent *pdirent;

	if (pdir == NULL)
		Err(path, err_str[bad_dir]);

	while (pdirent = readdir(pdir))
	{
		char *name = pdirent->d_name;
		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
			continue;

		SlurpTryName(name, path, keep_path);
	}

	int rc = closedir(pdir);
}



// TryArg()
//
// This tries an argument value to see if it is a directory or a file,
// and does the right thing with it.  For a directory argument, that
// means slurping (unless options.slurp_dir_arg is false).
//
// Arguments are saved in args_try_list, and this function is called
// once for each entry in the list.
//
// This is different from SlurpTryName() because: 0) SlurpTryName()
// never slurps a directory; 1) SlurpTryName() skips hidden files, and
// if the user went to the trouble to specify a file as an argument, we
// should list it even if it would normally be skipped as a hidden file.

inline void
TryArg(CSREF arg, bool keep_path)
{
	filetype ft;
	bool b = check_type(arg, ft);
	if (!b)
	{
		Err(arg, err_str[bad_filename]);
		return;
	}

	if (ft == ft_dir)
	{
		if (options.slurp_dir_arg)
			SlurpDir(arg, keep_path);
		else
			AddDir(arg);
	}
	else
		AddToMap(arg);
}



// TryArgList()
//
// Processes every saved argument in args_try_list.

void
TryArgList(bool keep_path)
{
	LIST_STRING::const_iterator p;
	for (p = args_try_list.begin(); p != args_try_list.end(); ++p)
		TryArg(*p, keep_path);
}



// NumericArg()
//
// Processes a numeric arg, and handles either a bad or missing arg.

int
NumericArg(int &i, int argc, ARGV argv, int min_val, err_strings err_msg)
{
	CSZ p;
	const int i_initial = i;

	// does current arg contain '=' followed by number?
	p = index(argv[i], ch_eq);
	if (p)
		++p;	// point one past '='
	else
	{
		// we need another arg; is there one more?
		if (i + 1 >= argc)
			ErrExit(argv[i], err_str[missing_arg]);

		p = argv[++i];
	}

	int t;
	bool b = ConvertAtoI(p, t);
	if (!b)	// conversion error
		ErrExit(argv[i_initial], err_str[err_msg]);

	if (t < min_val)
		ErrExit(argv[i_initial], err_str[err_msg]);

	return t;
}



// StringArg()
//
// Processes a string arg, and handles a missing arg.

string
StringArg(int &i, int argc, ARGV argv)
{
	CSZ p;

	// does current arg contain '=' followed by string?
	p = index(argv[i], ch_eq);
	if (p)
		++p;	// point one past '='
	else
	{
		// we need another arg; is there one more?
		if (i + 1 >= argc)
			ErrExit(argv[i], err_str[missing_arg]);

		p = argv[++i];
	}

	return string(p);
}



// ArgMatches()
//
// A convenience function that checks an argument to see if it matches
// either of two values: the short and long forms of a possible
// flag.  If takes_arg is true, this expects a follow-on argument, and
// if the user is using the long form, we need to look for --foo=bar form.

inline bool
NonEmptySz(CSZ csz)
{
	// returns true if csz is a non-NULL pointer to a non-empty string
	return csz && csz[0] != ch_nul;
}

bool
ArgMatches(CSZ arg, arg_strings arg_string_s, arg_strings arg_string_l,
		bool takes_arg)
{
	CSZ arg_s = arg_str[arg_string_s];
	CSZ arg_l = arg_str[arg_string_l];

	if (NonEmptySz(arg_s) && strcmp(arg, arg_s) == 0)
		return true;	// -X where X is short option

	if (takes_arg)
	{
		if (NonEmptySz(arg_l) && InitialMatch(arg_l, arg))
		{
			int len = strlen(arg_l);
			if (arg[len] == ch_eq)
				return true;	// --long-option=<whatever>
			else
				ErrExit(arg, err_str[bad_long_opt]);
		}
	}
	else
		if (NonEmptySz(arg_l) && strcmp(arg, arg_l) == 0)
			return true;	// --X where X is long option


	return false;
}



// DoSetOptions() and SetOptions()
//
// Loops through all command-line options.  Sets options flags
// accordingly.  Anything that doesn't match as a flag is appended to
// args_try_list for later.
//
// The actual work is done by DoSetOptions(), which processes all
// arguments it receives.  With the usual argc/argv that the main()
// function gets, the program name will be the first option, so
// SetOptions() skips that first option and then calls DoSetOptions().
//
// Call SetOptions() when you want the first arg skipped, and call
// DoSetOptions() when you don't.

void
DoSetOptions(int argc, ARGV argv)
{
	for (int i = 0; i < argc; ++i)
	{
		CSZ arg = argv[i];

		if (ArgMatches(arg, help_s, help_l, false))
			PrintStrings(cout, usage_strings, usage_strings_max, 0);

		else if (ArgMatches(arg, license_s, license_l, false))
			PrintStrings(cout, license_strings, license_strings_max, 0);

		else if (ArgMatches(arg, version_s, version_l, false))
			PrintStrings(cout, version_strings, version_strings_max, 0);

		else if (ArgMatches(arg, ascii_s, ascii_l, false))
			options.sort = sort_ascii;

		else if (ArgMatches(arg, ascii_ic_s, ascii_ic_l, false))
			options.sort = sort_ascii_ic;

		else if (ArgMatches(arg, locale_s, locale_l, false))
			options.sort = sort_locale;

		else if (ArgMatches(arg, dir_s, dir_l, false))
			options.slurp_dir_arg = false;

		else if (ArgMatches(arg, ext_width_s, ext_width_l, true))
			options.ext_width = NumericArg(i, argc, argv, 1, need_ge_one);

		else if (ArgMatches(arg, ext_limit_s, ext_limit_l, true))
			options.ext_limit = NumericArg(i, argc, argv, 1, need_ge_one);

		else if (ArgMatches(arg, force_lower_s, force_lower_l, false))
			options.force_lower = true;

		else if (ArgMatches(arg, line_width_s, line_width_l, true))
			options.line_width = NumericArg(i, argc, argv, 1, need_ge_one);

		else if (ArgMatches(arg, margin_s, margin_l, true))
			options.line_margin = NumericArg(i, argc, argv, 0, need_ge_zero);

		else if (ArgMatches(arg, show_all_s, show_all_l, false))
			options.show_all = true;

		else if (ArgMatches(arg, name_sep_s, name_sep_l, true))
			options.name_separator = StringArg(i, argc, argv);

		else if (ArgMatches(arg, repl_spaces_s, repl_spaces_l, true))
		{
			options.s_replace_space = StringArg(i, argc, argv);
			if (options.s_replace_space.compare(s_space) == 0)
				options.replace_spaces = false;	// replacement is a space!
			else
				options.replace_spaces = true;
		}

		else if (ArgMatches(arg, name_sep_s, name_sep_l, true))
			options.name_separator = StringArg(i, argc, argv);

		else if (ArgMatches(arg, verbose_s, verbose_l, true))
			options.verbose_level = NumericArg(i, argc, argv, 0, need_ge_zero);

		else
			// if it's not a command-line switch, try it as a file argument
			args_try_list.push_back(arg);
	}
}



void
SetOptions(int argc, ARGV argv)
{
	if (argc <= 0)
		return;

	options.progname = argv[0];

	DoSetOptions(argc - 1, argv + 1);	// skip argv[0]
}



// ReadEnvOptions()
//
// Checks for an environment variable called LFOPTS; if it's found,
// parses it for options and sets them accordingly.
//
// Builds an "argv" array, and passes it to DoSetOptions().  Thus the
// argument processing is identical to the command-line argument
// processing, as it's done by the same code.

void
ReadEnvOptions()
{
	CSZ lfopts = getenv(LF_LFOPTS);

	if (lfopts == NULL)
		return;

	options.lfopts = lfopts;

	wordexp w(lfopts);

	if (w.success())
		DoSetOptions(w.argc(), w.argv());
	else
		ErrExit(err_str[bad_lfopts]);
}



// PrintReportAboutOptions()
//
// Used when the user requests verbose output.

void
PrintReportAboutOptions()
{
	if (options.lfopts.length() > 0)
		cout << verbose_str[v_lfopts] << "[" << options.lfopts << "]" << endl;

	if (options.sort == sort_ascii)
		cout << verbose_str[v_sort_ascii] << endl;
	else if (options.sort == sort_ascii_ic)
		cout << verbose_str[v_sort_ascii_ic] << endl;
	else 
	{
		Assert(options.sort == sort_locale);
		cout << verbose_str[v_sort_locale] << def_locale.name() << endl;
	}
	
	cout << verbose_str[v_line_width] << options.line_width << endl;
	if (options.line_margin != 0)
		cout << verbose_str[v_margin] << options.line_margin << endl;
	cout << verbose_str[v_ext_limit] << options.ext_limit << endl;
	cout << verbose_str[v_ext_width] << options.ext_width << endl;

	if (options.name_separator.compare(s_space) != 0)
		cout << verbose_str[v_name_sep] << "["
				<< options.name_separator << "]" << endl;
	if (options.replace_spaces)
		cout << verbose_str[v_repl_spaces] << "["
				<< options.s_replace_space << "]" << endl;

	if (options.force_lower)
		cout << verbose_str[v_force_lower] << endl;
	if (options.show_all)
		cout << verbose_str[v_show_all] << endl;
	if (options.slurp_dir_arg == false)
		cout << verbose_str[v_dir] << endl;

	cout << endl;
}



// LenPrint()
//
// Prints a string, with an optional minimum width; then returns how
// many characters were printed.

int
LenPrint(CSZ s, int min_width = 0)
{
	int len = strlen(s);

	if (min_width && min_width > len)
	{
		cout.width(min_width);
		len = min_width;
	}

	cout << s;

	return len;
}



int
LenPrint(CSREF s, int min_width = 0)
{
	int len = s.length();

	if (min_width && min_width > len)
	{
		cout.width(min_width);
		len = min_width;
	}

	cout << s;

	return len;
}



// PrintBasenames()
//
// Prints basenames on a line.  The width argument is passed in so it
// knows how much of the current line was already printed (to print the
// extension).
//
// Tries not to break basenames across multiple lines, but for a
// basename that is longer than the available line width, it has to.

void
PrintBasenames(int width, const PSET_STRING pset_basenames)
{
	const int gap_width = options.ext_width + options.ext_separator.length();

	bool need_separator = false;
	// We don't need to print a name separator until after we have printed
	// at least one basename on any line.

	SET_STRING::const_iterator p;
	for (p = pset_basenames->begin(); p != pset_basenames->end(); ++p)
	{
		int len = p->length();
		if (need_separator)
			len += options.name_separator.length();

		// if file name is too long for current line...
		// ...and we would get more room with a new line...
		// ... then start a new line.  If width == gap_width
		// or even somehow got shorter than gap_width, no point in
		// starting a new line.

		if (width + len > options.width() && width > gap_width)
		{
			cout << endl;	// start new line
			width = 0;

			// print gap to make basename lines line up
			width += LenPrint("", gap_width);

			need_separator = false;	// new line; don't need a sep yet
		}

		if (need_separator)
			width += LenPrint(options.name_separator);

		width += LenPrint(*p);
		need_separator = true;
	}

	cout << endl;
}



// PrintFilenames()
//
// Print all filenames in the terse format.

void
PrintFilenames()
{
	SET_STRING::const_iterator p;
	int width;

	if (dirs_set.size() > 0)	// any dirs saved in set?
	{
		// we have dirs to output, so output the DIRS line at top
		width = 0;
		width += LenPrint(dirs_str, options.ext_width);
		width += LenPrint(options.ext_separator);

		PrintBasenames(width, &dirs_set);
	}

	for (p = ext_set.begin(); p != ext_set.end(); ++p)
	{
		width = 0;
		width += LenPrint(*p, options.ext_width);
		width += LenPrint(options.ext_separator);

		PrintBasenames(width, ext_map[*p]);
	}
}



// main()
//
// Main function.

int
main(int argc, ARGV argv)
{
	SetStrings();
#ifdef DEBUG
	CheckStrings();
#endif // DEBUG

	ReadEnvOptions();
	SetOptions(argc, argv);

	// Call InitLocale() *after* argument processing; see the comments
	// at the declaration of InitLocale() for an explanation why.
	InitLocale();	

	if (options.verbose_level >= 2)
		PrintReportAboutOptions();

	string cwd = GetCwd();
	if (args_try_list.size() == 0)
	{
		// default: list current directory

		if (options.slurp_dir_arg && options.verbose_level >= 1)
			cout << verbose_str[v_list_in_dir] << cwd << endl;
		TryArg(cwd, false);
	}
	else if (args_try_list.size() == 1)
	{
		// If user specifies just one arg, and it's a dir, slurp it.
		// If it's not a dir or we're not slurping, we will list it.
		// If it's a relative path, print the cwd at the top.
		CSREF arg = *(args_try_list.begin());
		filetype ft;
		bool b = check_type(arg, ft);
		if (options.verbose_level >= 1)
		{
			if (b && ft == ft_dir && options.slurp_dir_arg)
				cout << verbose_str[v_list_in_dir] << arg << endl;
			else if (!IsAbsolutePath(arg))
				cout << verbose_str[v_list_in_dir] << cwd << endl;
		}

		// List it.  If it is a directory it will be slurped without path.
		TryArgList(false);
	}
	else
	{
		// Multiple args.  If they are directories, they will be slurped
		// with paths included.

		if (options.verbose_level >= 1)
			cout << verbose_str[v_list_in_dir] << cwd << endl;
		TryArgList(true);
	}

	PrintFilenames();

	return 0;
}
