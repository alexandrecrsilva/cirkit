/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "read_aigmeta.hpp"

#include <iostream>

#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

namespace revkit
{

using namespace boost::assign;
using boost::property_tree::ptree;

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

inline void read_port_def( aigmeta_box& box, const ptree& pt )
{
  aigmeta_port port;

  port.id = pt.get<unsigned>( "bundle_id" );
  port.name = pt.get<std::string>( "port_name" );

  box.ports += port;
}

inline void read_box_def( aigmeta& meta, const ptree& pt )
{
  aigmeta_box box;

  box.id = pt.get<unsigned>( "output_bundle_id" );
  box.oper_type = pt.get<std::string>( "oper_type" );

  for ( const auto& port_def : pt.get_child( "port_def" ) )
  {
    read_port_def( box, port_def.second );
  }

  meta.boxes += box;
}

inline void read_bundle_def( aigmeta& meta, const ptree& pt )
{
  using boost::adaptors::transformed;

  aigmeta_bundle bundle;

  bundle.id = pt.get<unsigned>( "bundle_id" );
  bundle.name = pt.get<std::string>( "name" );
  boost::push_back( bundle.literals, pt.get_child( "litx" ) | transformed( []( const ptree::value_type& v ) { return boost::lexical_cast<unsigned>( v.second.data() ); } ) );

  meta.bundles += bundle;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void read_aigmeta( aigmeta& meta, const std::string& filename )
{
  ptree pt;

  read_json( filename, pt );

  meta.dut = pt.get<std::string>( "dut" );

  for ( const auto& box_def : pt.get_child( "box_def" ) )
  {
    read_box_def( meta, box_def.second );
  }

  for ( const auto& bundle_def : pt.get_child( "bundle_def" ) )
  {
    read_bundle_def( meta, bundle_def.second );
  }
}

std::ostream& operator<<( std::ostream& os, const aigmeta& meta )
{
  os << "DUT:      " << meta.dut << std::endl
     << "#Boxes:   " << meta.boxes.size() << std::endl
     << "#Bundles: " << meta.bundles.size() << std::endl;

  return os;
}

}

// Local Variables:
// c-basic-offset: 2
// End: