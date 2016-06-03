/*  Created on: Jun 3, 2016
 *      Author: T. Delame (tdelame@gmail.com) */
BEGIN_MP_NAMESPACE

static const uint8_t atom_links_property_index = 0;
static const uint8_t atom_faces_property_index = 1;
static const uint8_t link_faces_property_index = 0;

template< typename atom_processer >
  void
  median_skeleton::process_atoms(
    atom_processer&& function, bool parallel )
  {
    if( parallel )
      {
# pragma omp parallel for schedule(dynamic)
        for( atom_index i = 0; i < m_impl->m_atoms_size; ++i )
          {
            function( m_impl->m_atoms[i] );
          }
      }
    else
      {
        for( atom_index i = 0; i < m_impl->m_atoms_size; ++i )
          {
            function( m_impl->m_atoms[i] );
          }
      }
  }

template< typename atom_filter >
  void
  median_skeleton::remove_atoms(
    atom_filter&& filter, bool parallel )
  {
    /* When an atom is removed:
     *   all its links and faces should be removed
     *     - this is job of remove_link_indices(...), which guarantees
     *     that the tagged links no longer exist, either in the tight
     *     buffer or in topology properties. Since a face is destroyed
     *     once one of its links is removed, this is sufficient to call
     *     that function.
     *   it must be exchanged with the last atom A of the tight buffer
     *     - its handle will not be valid anymore
     *     - the handle of A need to point to the new memory
     *     - the index to handle index of A should be updated
     */

    const atom_index atom_size = m_impl->m_atoms_size;
    /* a flag buffer is necessary if the filter function relies on reference
     * to atoms or atom index. For example, if the filter function remove
     * atoms with indices 1 and 3, with 4 atoms in total in the buffer. When
     * atom #1 is removed, the atom #3 is move into it. Thus, when the former
     * atom #3 (now #1) is tested, the filter function will not recognize it. */
    bool* flags = new bool[atom_size];
    if( parallel )
      {
# pragma omp parallel for schedule(dynamic)
        for( atom_index i = 0; i < atom_size; ++i )
          {
            flags[i] = filter( m_impl->m_atoms[i] );
          }
      }
    else
      {
        for( atom_index i = 0; i < atom_size; ++i )
          {
            flags[i] = filter( m_impl->m_atoms[i] );
          }
      }

      {
        const link_index link_size = m_impl->m_links_size;
        bool* link_flags = new bool[link_size];
# pragma omp parallel for
        for( link_index i = 0; i < link_size; ++i )
          link_flags[i] = false;
# pragma omp parallel for
        for( atom_index i = 0; i < atom_size; ++i )
          {
            if( flags[i] )
              {
                auto& alinks =
                    m_impl->m_atom_properties[atom_links_property_index]->get
                        < atom_links_property > (i);
                for( auto& alink : alinks )
                  {
                    link_flags[m_impl->m_link_handles[alink.first.index].link_index] =
                        true;
                  }
              }
          }
        remove_link_indices( link_flags );
      }

    /* INVARIANT:
     * - all atoms with indices in [[0, left[[ are not removed
     * - all atoms with indices in [[left, right[[ need to be checked
     * - all atoms with indices in [[right, size[[ should be cleaned (all
     * objects referring to an atom should be destroyed)
     * Thus, in the end (i.e. when left == right ), we have:
     * - atoms with indices in [[0,left[[ are clean and tight
     * - atoms with indices in [[left, size[[ should be cleaned
     * - left is the new size of atoms
     */
    atom_index next_free_slot = m_impl->m_atoms_next_free_handle_slot;
    atom_index left = 0, right = atom_size;
    while( left < right )
      {
        if( flags[left] )
          {
            // update the left_entry
            const auto left_entry_index =
                m_impl->m_atom_index_to_handle_index[left];
            auto left_entry = m_impl->m_atom_handles + left_entry_index;
            left_entry->next_free_index = next_free_slot;
            left_entry->status = datastructure::STATUS_FREE;
            next_free_slot = left_entry_index;

            // look for a valid atom in the end of the buffer
            --right;
            while( left < right && flags[right] )
              {
                const auto entry_index =
                    m_impl->m_atom_index_to_handle_index[right];
                auto entry = m_impl->m_atom_handles + entry_index;
                entry->next_free_index = next_free_slot;
                entry->status = datastructure::STATUS_FREE;
                next_free_slot = entry_index;
                --right;
              }

            if( left != right )
              {
                // move the last atom into this one to keep the buffer tight
                m_impl->m_atoms[left] = std::move( m_impl->m_atoms[right] );
                for( auto& property : m_impl->m_atom_properties )
                  property->move( right, left );

                const auto right_entry_index =
                    m_impl->m_atom_index_to_handle_index[right];
                auto right_entry = m_impl->m_atom_handles + right_entry_index;
                right_entry->atom_index = left;
                m_impl->m_atom_index_to_handle_index[left] = right_entry_index;
                ++left;
              }
          }
        else
          ++left;
      }
    delete[] flags;

    // destroy properties of atoms in [[left, size[[
    for( auto& property : m_impl->m_atom_properties )
      property->destroy( left, atom_size );

    m_impl->m_atoms_size = left;
    m_impl->m_atoms_next_free_handle_slot = next_free_slot;
  }

template< typename link_processer >
void
median_skeleton::process_links(
  link_processer&& function, bool parallel )
{
  if( parallel )
    {
# pragma omp parallel for schedule(dynamic)
      for( link_index i = 0; i < m_impl->m_links_size; ++i )
        {
          function( m_impl->m_links[i] );
        }
    }
  else
    {
      for( link_index i = 0; i < m_impl->m_links_size; ++i )
        {
          function( m_impl->m_links[i] );
        }
    }
}

template< typename link_filter >
void
median_skeleton::remove_links(
  link_filter&& filter, bool parallel )
{
  const link_index size = m_impl->m_links_size;
  /* a flag buffer is necessary if the filter function relies on reference
   * to links or link index. For example, if the filter function remove
   * links with indices 1 and 3, with 4 links in total in the buffer. When
   * link #1 is removed, the link #3 is move into it. Thus, when the former
   * link #3 (now #1) is tested, the filter function will not recognize it. */
  bool* flags = new bool[size];
  if( parallel )
    {
# pragma omp parallel for schedule(dynamic)
      for( link_index i = 0; i < size; ++i )
        {
          flags[i] = filter( m_impl->m_links[i] );
        }
    }
  else
    {
      for( link_index i = 0; i < size; ++i )
        {
          flags[i] = filter( m_impl->m_links[i] );
        }
    }

  remove_link_indices( flags );
}

template< typename face_processer >
void
median_skeleton::process_faces(
  face_processer&& function, bool parallel )
{
  if( parallel )
    {
# pragma omp parallel for schedule(dynamic)
      for( face_index i = 0; i < m_impl->m_faces_size; ++i )
        {
          function( m_impl->m_faces[i] );
        }
    }
  else
    {
      for( face_index i = 0; i < m_impl->m_faces_size; ++i )
        {
          function( m_impl->m_faces[i] );
        }
    }
}

template< typename face_filter >
void
median_skeleton::remove_faces(
  face_filter&& filter, bool parallel )
{
  const face_index size = m_impl->m_faces_size;
  /* a flag buffer is necessary if the filter function relies on reference
   * to faces or face index. For example, if the filter function remove
   * faces with indices 1 and 3, with 4 faces in total in the buffer. When
   * face #1 is removed, the face #3 is move into it. Thus, when the former
   * face #3 (now #1) is tested, the filter function will not recognize it. */
  bool* flags = new bool[size];
  if( parallel )
    {
# pragma omp parallel for schedule(dynamic)
      for( face_index i = 0; i < size; ++i )
        {
          flags[i] = filter( m_impl->m_faces[i] );
        }
    }
  else
    {
      for( face_index i = 0; i < size; ++i )
        {
          flags[i] = filter( m_impl->m_faces[i] );
        }
    }

  remove_faces_indices( flags );
}

END_MP_NAMESPACE
