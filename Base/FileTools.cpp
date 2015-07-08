/**
 * \file FileTools.h
 * 26/4/2010 LB Initial implementation
 *
 */

#include "FileTools.h"

#include <iostream>
#include <fstream>
#include <cstdio>

#include <sys/stat.h>

// for getCwd
#ifdef WINDOWS
#include <direct.h>
#else
#include <unistd.h>
#endif


/**
 * Returns true if given file exists. From http://www.techbytes.ca/techbyte103.html
 */
bool IsFileExisting(std::string const& strFilename)
{
	struct stat stFileInfo;
	bool blnReturn;
	int intStat;

	// Attempt to get the file attributes
	intStat = stat(strFilename.c_str(),&stFileInfo);

	if(intStat == 0)
		// We were able to get the file attributes
		// so the file obviously exists.
		blnReturn = true;
	else
		// We were not able to get the file attributes.
		// This may mean that we don't have permission to
		// access the folder which contains this file. If you
		// need to do that level of checking, lookup the
		// return values of stat which will give you
		// more details on why stat failed.
		blnReturn = false;

	return blnReturn;
}

bool HasCRInLineEnding(std::string const& strFilename)
{
	std::ifstream is(strFilename.c_str(), std::ios::in | std::ios::binary);
	if (!is) {
		std::cout << "*** error: could not open " << strFilename.data() << std::endl;
		return false;
	}

	bool foundCR = false;
	while (is.good()) {
		char c;
		is.read(&c, sizeof(c));
		if (c == '\r') {
			foundCR = true;
			break;
		}
		else if (c == EOF || c == '\n') {
			break;
		}
	}

	is.close();

	return foundCR;
}


inline char getDirSep()
{
#ifdef WINDOWS
    return '\\';
#else
    return '/';
#endif
}


/**
 * @brief removes all occurences of c from the beginning of str
 */
std::string ltrim(const std::string& str, const char c)
{
	const size_t idx = str.find_first_not_of(c);
	if (idx == std::string::npos) {
		// string consists only of c
		return "";
	} else {
		return str.substr(idx);
	}
}


/**
 * @brief removes all occurences of c from the end of str
 */
std::string rtrim(const std::string& str, const char c)
{
	const size_t idx = str.find_last_not_of(c);
	if (idx == std::string::npos) {
		// string consists only of c
		return "";
	} else {
		return str.substr(0, idx+1);
	}
}


std::string pathJoin(const std::string& path1, const std::string& path2)
{
	std::cerr << "@@@@:" << __FILE__ << ":" << __LINE__ << ":" << path1 << "<@@@>" << path2 << std::endl;

	if (path1.empty()) return path2;
	if (path2.empty()) return path1;

	const char dirSep = getDirSep();

	const std::string s = rtrim(path1, dirSep) + dirSep + ltrim(path2, dirSep);

	std::cerr << "@@@@:" << __FILE__ << ":" << __LINE__ << ":" << s << std::endl;

	return s;
}


std::string pathBasename(const std::string& path)
{
	if (path.length() == 0) return path;

	const char dirSep = getDirSep();

	// ignore trailing / or \ respectively
	const size_t idxNsep = path.find_last_not_of(dirSep);
	if (idxNsep == std::string::npos) {
		// path contains only dirSep characters
		return std::string(1, dirSep);
	}

	const size_t idx = path.find_last_of(dirSep, idxNsep);
	std::string s;

	if (idx != std::string::npos) {
		s = path.substr(idx+1, idxNsep-idx);
	} else {
		s = path.substr(0, idxNsep+1);
	}

	return s;
}


std::string getCwd()
{
    char cwd[FILENAME_MAX];

#ifdef WINDOWS
    _getcwd(cwd, FILENAME_MAX);
#else
    getcwd(cwd, FILENAME_MAX);
#endif

    return cwd;
}
