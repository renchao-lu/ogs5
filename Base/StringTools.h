#ifndef STRINGTOOLS_H
#define STRINGTOOLS_H

#include "Configure.h"

#include <string>
#include <list>
#include <sstream>
#include <fstream>
#include <iostream>
#include <ctype.h>


/**
 *   Splits a string into a list of strings.
 *  \param str String to be splitted
 *  \param delim Character indicating that the string should be splitted
 *  \return
 */
static std::list<std::string> splitString(const std::string &str, char delim)
{
	std::list<std::string> strList;
	std::stringstream ss(str);
    std::string item;
    while(getline(ss, item, delim)) {
        strList.push_back(item);
    }
    return strList;
}

/**
 *   Replaces a substring with another in a string
 *  \param searchString Search for this string
 *  \param replaceString Replace with this string
 *  \param stringToReplace Search and replace in this string
 *  \return The modified string
 */
static std::string replaceString(const std::string &searchString, const std::string &replaceString, std::string stringToReplace)
 {
	std::string::size_type pos = stringToReplace.find(searchString, 0);
	int intLengthSearch = searchString.length();

	while (std::string::npos != pos) {
		stringToReplace.replace(pos, intLengthSearch, replaceString);
		pos = stringToReplace.find(searchString, 0);
	}
	return stringToReplace;
}

/**
 *   Converts a number (double, float, int, ...) into a string
 *  \param d The number to be converted
 *  \return The number as string
 */
template<typename T> std::string number2str(T d)
{
	std::stringstream out;
	out << d;
	return out.str();
}

/**
 *   Converts a string into a number (double, float, int, ...)
 *  Example: size_t number (str2number<size_t> (str));
 *  \param str string to be converted
 *  \return the number
 */
template<typename T> T str2number (const std::string &str)
{
	std::stringstream strs (str, std::stringstream::in | std::stringstream::out);
	T v;
	strs >> v;
	return v;
}


static void trim(std::string &str)
{
	std::string::size_type pos = str.find_last_not_of(' ');
	if(pos != std::string::npos) 
	{
		str.erase(pos + 1);
		pos = str.find_first_not_of(' ');
		if(pos != std::string::npos) str.erase(0, pos);
	}
	else str.erase(str.begin(), str.end());
}

static void correctScientificNotation(std::string filename, size_t precision = 0)
{
#ifdef MSVC
	std::ifstream stream;
	std::ofstream outputStream;
	
	stream.open(filename.c_str());
	std::string tmpFilename = filename + ".tmp";
	outputStream.open(tmpFilename.c_str());

	if (!stream)
	{
		std::cout << "correctScientificNotation: fstream is not open" << std::endl;
		return;
	}

	std::string line;
	
	// Iterate over lines in stream
	while (getline(stream, line))
	{
		std::string word;
		std::istringstream iss(line);
		// Iterate over all words in line
		while (iss >> word)
		{
			// Search for e+0
			std::size_t exponentPosition = word.find("e+0", precision);
			if (exponentPosition == std::string::npos)
				// If not found search for e-0
				exponentPosition = word.find("e-0", precision);
			if (exponentPosition != std::string::npos)
			{
				std::size_t wordSize = word.size();
				std::size_t exponentSize = wordSize - exponentPosition;

				if(exponentSize > 4)
				{
					// Erase the leading zero considering trailing characters
					int i = wordSize - 1;
					while (!isdigit(word[i]))
						--i;
					
					size_t erasePos = wordSize - 3 - (wordSize - 1 - i);
					std::string eraseString = word.substr(erasePos, 1);
					if (eraseString.find("0") != std::string::npos)
						word.erase(erasePos, 1);
				}
			}

			outputStream << word << " ";
		}
		outputStream << std::endl;
	}

	stream.close();
	outputStream.close();

	remove(filename.c_str());
	rename(tmpFilename.c_str(), filename.c_str());

#endif
}

#endif //STRINGTOOLS_H
