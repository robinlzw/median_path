/*  Created on: Mar 11, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */

# ifndef MEDIAN_PATH_IO_UTILITIES_H_
# define MEDIAN_PATH_IO_UTILITIES_H_
# include "../io.h"
# include <fstream>
# include <algorithm>

BEGIN_MP_NAMESPACE

  /**@brief Get the next relevant line of a skeleton file.
   *
   * In a skeleton file, this function fetch the next relevant line.
   * That is a line that is non empty once we removed the comments.
   * If no such line exist, it means we have reached the end of the file
   * and this function returns false.
   * @param input Input file stream.
   * @param line_number Store the number of the last relevant line read and will
   * be updated to contain the line number of the next relevant line read if one
   * such line is found.
   * @param line Will contain the next relevant line if such a line is found.
   * @return True if another relevant line is found.
   */
  bool
  get_next_relevant_line( std::ifstream& input, size_t& line_number, std::string& line )
  {
    do
      {
        if( input.eof() || input.fail() )
          return false;
        getline( input, line );
        ++line_number;

        // remove a comment, which starts by the sequence "//"
        size_t found = line.find( "//" );
        if( found != std::string::npos )
          line = line.substr(0, found );

        // remove leading spaces
        line.erase(
            line.begin(),
            // first non space character
            std::find_if_not(
                line.begin(),
                line.end(),
                [](int c){return std::isspace<char>(c , std::locale::classic() );}));

        // remove trailing characters
        line.erase(
            // last non space character
            std::find_if_not(
                line.rbegin(),
                line.rend(),
                [](int c){return std::isspace<char>(c, std::locale::classic() );}).base(),
            line.end());
      }
    while( line.empty() );
    return true;
  }
END_MP_NAMESPACE
# endif
