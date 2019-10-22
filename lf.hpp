// lf.hpp
//
// This declares all the strings used by lf.cpp, but does not define them.
// It includes the various language headers that do define them for the
// various languages: lf_en.hpp, lf_fr.hpp, etc.
//
// It does declare a few important strings as #defines, such as the
// version string.  These may be expanded wherever they are needed.
// They are here so they will all be together in exactly one place.
//
// Author: Steve R. Hastings <steve@hastings.org>


#ifndef LF_HPP

#define LF_HPP


#include "util.hpp"



#define LF_CANONICAL_NAME "lf"

#define LF_VERSION_STR "0.7.5"
#define LF_COPYRIGHT_YEAR "2006"

#define LF_EMAIL "lf@langri.com"
#define LF_LFOPTS "LFOPTS"



enum arg_strings
{
	help_s, help_l,
	license_s, license_l,
	version_s, version_l,
	ascii_s, ascii_l,
	ascii_ic_s, ascii_ic_l,
	locale_s, locale_l,
	dir_s, dir_l,
	ext_width_s, ext_width_l,
	ext_limit_s, ext_limit_l,
	force_lower_s, force_lower_l,
	line_width_s, line_width_l,
	margin_s, margin_l,
	show_all_s, show_all_l,
	name_sep_s, name_sep_l,
	repl_spaces_s, repl_spaces_l,
	verbose_s, verbose_l,
	ARG_STRINGS_MAX,
};

CSZ arg_str[ARG_STRINGS_MAX];



enum verbose_strings
{
	v_list_in_dir,
	v_lfopts,
	v_sort_ascii,
	v_sort_ascii_ic,
	v_sort_locale,
	v_line_width,
	v_margin,
	v_name_sep,
	v_repl_spaces,
	v_force_lower,
	v_show_all,
	v_dir,
	v_ext_limit,
	v_ext_width,
	VERBOSE_STRINGS_MAX,
};

CSZ verbose_str[VERBOSE_STRINGS_MAX];



enum err_strings
{
	bad_dir,
	bad_long_opt,
	missing_arg,
	need_ge_one,
	need_ge_zero,
	bad_filename,
	bad_locale,
	bad_lfopts,
	ERR_STRINGS_MAX,
};

CSZ err_str[ERR_STRINGS_MAX];



CSZ const* usage_strings;
int usage_strings_max = 0;

CSZ const* version_strings;
int version_strings_max = 0;

CSZ const* license_strings;
int license_strings_max = 0;

CSZ dirs_str = "////";



#include "lang/en/lf_strings.hpp"
#include "lang/fr/lf_strings.hpp"



#endif // LF_HPP
