namespace median_path {

dts_definition()::atom_handle_entry::atom_handle_entry()
  : next_free_index{ 0 }, counter{ 0 },
	status{ STATUS_FREE }, atom_index{ 0 }
{}

dts_definition()::link_handle_entry::link_handle_entry()
  : next_free_index{ 0 }, counter{ 0 },
    status{ STATUS_FREE }, link_index{ 0 }
{}

dts_definition()::face_handle_entry::face_handle_entry()
  : next_free_index{ 0 }, counter{ 0 },
    status{ STATUS_FREE }, face_index{ 0 }
{}

dts_definition()::atom_handle::atom_handle()
  : index{ max_atom_handle_index },
    counter{ max_atom_handle_counter + 1 }
{}

dts_definition()::atom_handle::atom_handle( atom_handle_type idx, atom_handle_type ctr )
  : index{ idx },
	counter{ ctr }
{}

dts_definition(inline)::atom_handle::operator atom_handle_type() const noexcept
{
  return (atom_handle_type(counter) << atom_handle_index_bits ) | index;
}

dts_definition(inline bool)::atom_handle::is_valid() const noexcept
{
  return counter <= max_atom_handle_counter;
}

dts_definition()::link_handle::link_handle()
  : index{ max_link_handle_index },
    counter{ max_link_handle_counter + 1 }
{}

dts_definition()::link_handle::link_handle( link_handle_type idx, link_handle_type ctr )
  : index{ idx },
    counter{ ctr }
{}

dts_definition(inline)::link_handle::operator link_handle_type() const noexcept
{
  return (link_handle_type(counter) << link_handle_index_bits ) | index;
}

dts_definition(inline bool)::link_handle::is_valid() const noexcept
{
  return counter <= max_link_handle_counter;
}


dts_definition()::face_handle::face_handle()
  : index{ max_face_handle_index },
    counter{ max_face_handle_counter + 1 }
{}

dts_definition()::face_handle::face_handle( face_handle_type idx, face_handle_type ctr )
  : index{ idx },
    counter{ ctr }
{}

dts_definition(inline)::face_handle::operator face_handle_type() const noexcept
{
  return (face_handle_type(counter) << face_handle_index_bits ) | index;
}

dts_definition(inline bool)::face_handle::is_valid() const noexcept
{
  return counter <= max_face_handle_counter;
}
}
