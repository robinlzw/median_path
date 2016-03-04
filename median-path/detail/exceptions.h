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
  struct skeleton_invalid_link_handle
      : public std::runtime_error
  {
    skeleton_invalid_link_handle( const std::string& file, size_t line )
      : std::runtime_error( "invalid link handle at line "
      + std::to_string( line ) + " of file " + file )
    {}
  };
  struct skeleton_invalid_face_handle
      : public std::runtime_error
  {
    skeleton_invalid_face_handle( const std::string& file, size_t line )
      : std::runtime_error( "invalid face handle at line "
      + std::to_string( line ) + " of file " + file )
    {}
  };

  struct skeleton_invalid_atom_pointer
      : public std::runtime_error
  {
    skeleton_invalid_atom_pointer( const std::string& file, size_t line )
      : std::runtime_error( "invalid atom pointer at line "
      + std::to_string( line ) + " of file " + file )
    {}
  };
  struct skeleton_invalid_link_pointer
      : public std::runtime_error
  {
    skeleton_invalid_link_pointer( const std::string& file, size_t line )
      : std::runtime_error( "invalid link pointer at line "
      + std::to_string( line ) + " of file " + file )
    {}
  };
  struct skeleton_invalid_face_pointer
      : public std::runtime_error
  {
    skeleton_invalid_face_pointer( const std::string& file, size_t line )
      : std::runtime_error( "invalid face pointer at line "
      + std::to_string( line ) + " of file " + file )
    {}
  };

  struct skeleton_invalid_atom_index
      : public std::runtime_error
  {
    skeleton_invalid_atom_index( const std::string& file, size_t line )
      : std::runtime_error( "invalid atom index at line "
      + std::to_string( line ) + " of file " + file )
    {}
  };
  struct skeleton_invalid_link_index
      : public std::runtime_error
  {
    skeleton_invalid_link_index( const std::string& file, size_t line )
      : std::runtime_error( "invalid link index at line "
      + std::to_string( line ) + " of file " + file )
    {}
  };
  struct skeleton_invalid_face_index
      : public std::runtime_error
  {
    skeleton_invalid_face_index( const std::string& file, size_t line )
      : std::runtime_error( "invalid face index at line "
      + std::to_string( line ) + " of file " + file )
    {}
  };

# define MP_THROW_EXCEPTION( name ) \
  throw name( __FILE__, __LINE__ )

END_MP_NAMESPACE
# endif 
