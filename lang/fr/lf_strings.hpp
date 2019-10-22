// This file defines all the strings used for the French language.


#ifndef LF_FR_HPP

#define LF_FR_HPP



namespace lang_fr
{

// NOTE: these translations were done with Babelfish.  I apologize to
// any French-speakers if they are bad.  Please send better ones to me at:
// lf@langri.com

CSZ const usage[] =
{
	"lf -- énumère des dossiers laconiquement, assorti par des prolongements.",
	"lf [option...] [filespec...]",
	"",
	"Les options:",
	"-h, --aide\t\tmontrez cette aide.",
	"--permis\t\tmontrez l'information sur le permis libre du lf.",
	"--version\t\tmontrez l'information de version.",
	"-a, --show-all\t\tshow all names, even ones normally hidden.",
	"-A, --ascii\t\tsort using ASCII order instead of current locale.",
	"-I, --ascii-ic\t\tlike --ascii, but ignores case.",
	"-L, --locale\t\tuse the default locale for sorting.",
	"-d, --directory\t\tprint directory arguments; do not list their files.",
	"-X n, --ext-width=n\tset default width of extensions to n.",
	"-Z n, --ext-limit=n\textensions wider than n are ignored.",
	"-F, --force-lower-case\tforce all file and directory names to lower case.",
	"-N s, --name-separator=s\tseparate names in listing with string s.",
	"-S s, --replace-spaces=s\treplace spaces in file names with string s.",
	"-w n, --line-width=n\tformat lines to be no longer than n characters.",
	"-M n, --margin=n\tleave blank n spaces at right of line.",
	"-v n, --verbose=n\tset verbosity level; 0 is least verbose.",
	"",
	"Report bugs to: " LF_EMAIL,
	"lf version " LF_VERSION_STR,
};



CSZ const version[] =
{
	LF_CANONICAL_NAME " " LF_VERSION_STR,
	"Written by Steve R. Hastings."
	"",
	"French translation provided by: Name",
	"",
	"Copyright (C) " LF_COPYRIGHT_YEAR " by Steve R. Hastings.",
	"",
	"This is free software; run with the --license option to see the license.",
	"There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A",
	"PARTICULAR PURPOSE.",
};



CSZ const license[] =
{
"This is the BSD license. For more information, see:",
"http://www.opensource.org/licenses/bsd-license.php",
"",
"Copyright (c) " LF_COPYRIGHT_YEAR " by Steve R. Hastings.",
"All rights reserved.",
"",
"Redistribution and use in source and binary forms, with or without",
"modification, are permitted provided that the following conditions are",
"met:",
"",
"    * Redistributions of source code must retain the above copyright",
"      notice, this list of conditions and the following disclaimer.",
"    * Redistributions in binary form must reproduce the above copyright",
"      notice, this list of conditions and the following disclaimer in",
"      the documentation and/or other materials provided with the",
"      distribution.",
"    * Neither the name of the author of this program nor the names of its",
"      contributors may be used to endorse or promote products derived",
"      from this software without specific prior written permission.",
"",
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS",
"IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED",
"TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A",
"PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER",
"OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,",
"EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,",
"PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR",
"PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF",
"LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING",
"NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS",
"SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.",
};

const int usage_max = sizeof(usage) / sizeof(CSZ);
const int version_max = sizeof(version) / sizeof(CSZ);
const int license_max = sizeof(license) / sizeof(CSZ);



void
SetStrings()
{
	arg_str[help_s] = "-h";
	arg_str[help_l] = "--aide";

	// The British spelling of "license" is accepted.
	// Languages with just one way to spell "license" can set one to NULL.
	arg_str[license_s] = "--permis";
	arg_str[license_l] = NULL;
	arg_str[version_s] = "--version";
	arg_str[version_l] = NULL;

	arg_str[ascii_s] = "-A";
	arg_str[ascii_l] = "--ascii";
	arg_str[ascii_ic_s] = "-I";
	arg_str[ascii_ic_l] = "--ascii-ic";
	arg_str[locale_s] = "-L";
	arg_str[locale_l] = "--locale";
	arg_str[dir_s] = "-d";
	arg_str[dir_l] = "--directory";
	arg_str[ext_width_s] = "-X";
	arg_str[ext_width_l] = "--ext-width";
	arg_str[ext_limit_s] = "-Z";
	arg_str[ext_limit_l] = "--ext-limit";
	arg_str[force_lower_s] = "-F";
	arg_str[force_lower_l] = "--force-lower-case";
	arg_str[line_width_s] = "-w";
	arg_str[line_width_l] = "--line-width";
	arg_str[margin_s] = "-M";
	arg_str[margin_l] = "--margin";
	arg_str[show_all_s] = "-a";
	arg_str[show_all_l] = "--show-all";
	arg_str[name_sep_s] = "-N";
	arg_str[name_sep_l] = "--name-separator";
	arg_str[repl_spaces_s] = "-S";
	arg_str[repl_spaces_l] = "--replace-spaces";
	arg_str[verbose_s] = "-v";
	arg_str[verbose_l] = "--verbose";

	verbose_str[v_list_in_dir] =
		"Listing files in: ";
	verbose_str[v_lfopts] =
		"LFOPTS is set to: ";
	verbose_str[v_sort_ascii] =
		"Sorting filenames using ASCII sort order.";
	verbose_str[v_sort_ascii_ic] =
		"Sorting filenames using case-insensitive ASCII sort order.";
	verbose_str[v_sort_locale] =
		"Sorting filenames using this locale: ";
	verbose_str[v_line_width] =
		"Formatting output for line width of: ";
	verbose_str[v_margin] =
		"Leaving a right margin with this many blank spaces: ";
	verbose_str[v_name_sep] =
		"Using this name separator: ";
	verbose_str[v_repl_spaces] =
		"Replacing spaces in file names with: ";
	verbose_str[v_force_lower] =
		"Forcing filenames to lower case.";
	verbose_str[v_show_all] =
		"Showing all files (including normally hidden ones).";
	verbose_str[v_dir] =
		"Showing directory name arguments instead of listing their files.";
	verbose_str[v_ext_limit] =
		"Ignoring extensions longer than: ";
	verbose_str[v_ext_width] =
		"Formatting extension display width of: ";

	err_str[bad_dir] =
		"could not open directory";
	err_str[bad_long_opt] =
		"option needs an argument, in this format: --option-name=<value>";
	err_str[missing_arg] =
		"option needs to be followed by an argument.";
	err_str[need_ge_one] =
		"option's argument must be a number >= 1.";
	err_str[need_ge_zero] =
		"option's argument must be a number >= 0.";
	err_str[bad_filename] =
		"no such file or directory";
	err_str[bad_locale] =
		"unable to detect locale; defaulting to ASCII sort.";
	err_str[bad_lfopts] =
		"unable to parse the LFOPTS environment variable.";


	dirs_str = "DIRS";

	usage_strings = usage;
	usage_strings_max = usage_max;
	version_strings = version;
	version_strings_max = version_max;
	license_strings = license;
	license_strings_max = license_max;
}


} // end of namespace



#endif
