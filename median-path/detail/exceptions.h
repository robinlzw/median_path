/* Created on: Mar 3, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_EXCEPTIONS_H_
# define MEDIAN_PATH_EXCEPTIONS_H_

# include "../median_path.h"
# include <stdexcept>

BEGIN_MP_NAMESPACE

  struct skeleton_atom_buffer_overflow
      : public std::runtime_error
  {
    skeleton_atom_buffer_overflow( const std::string& file, size_t line )
      : std::runtime_error( "cannot add more atom to buffer at line "
      + std::to_string( line ) + " of file " + file )
    {}
  };
  struct skeleton_link_buffer_overflow
      : public std::runtime_error
  {
    skeleton_link_buffer_overflow( const std::string& file, size_t line )
      : std::runtime_error( "cannot add more link to buffer at line "
      + std::to_string( line ) + " of file " + file )
    {}
  };
  struct skeleton_face_buffer_overflow
      : public std::runtime_error
  {
    skeleton_face_buffer_overflow( const std::string& file, size_t line )
      : std::runtime_error( "cannot add more face to buffer at line "
      + std::to_string( line ) + " of file " + file )
    {}
  };

  struct skeleton_invalid_atom_handle
      : public std::runtime_error
  {
    skeleton_invalid_atom_handle( const std::string& file, size_t line )
      : std::runtime_error( "invalid atom handle at line "
      + std::to_string( line ) + " of file " + file )
    {}
  };


# define MP_THROW_EXCEPTION( name ) \
  throw name( __FILE__, __LINE__ )

END_MP_NAMESPACE
# endif 
