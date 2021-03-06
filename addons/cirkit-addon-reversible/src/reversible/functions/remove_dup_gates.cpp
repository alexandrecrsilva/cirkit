/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "remove_dup_gates.hpp"

#include <reversible/gate.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/pauli_tags.hpp>
#include <reversible/rotation_tags.hpp>

namespace cirkit
{

    
/* Also join adjacent T or T* gates
 */
circuit remove_dup_gates( const circuit& circ )
{
    circuit result = circ;
    gate g;
    unsigned i = 0, j;
    while(i < result.num_gates() - 1 )
    {
        j = i + 1;
        bool done = false;
        bool incr_i = true;
        while( ( !done ) && ( j < result.num_gates() ) )
        {
            if( is_inverse( result[i], result[j] ) )
            {
//                std::cout << "is_inverse " << i << " " << j << "\n";
                result.remove_gate_at(j);
                result.remove_gate_at(i);
                done = true;
                if(i>10)
                    i = i - 10;
                else
                    i = 0; 
                incr_i = false;
            }
            if ( !done && gates_can_merge( result[i], result[j], g) )
            {
 //               std::cout << "gates_can_merge " << i << " " << j << "\n";
                result.remove_gate_at(j);
                result[i] = g;
                done = true;
                if(i>10)
                    i = i - 10;
                else
                    i = 0; 
                incr_i = false;
            }
            if(!done && gates_can_move( result[i], result[j]) )
            {
//                std::cout << "gates_can_move " << i << " " << j << "\n";
                j++;
            }
            else{
                done = true;
            }
        }
        if ( incr_i )
        {
            i++;
        }
    }
    return result;
}

// check if two gates are inverse of each other
// only consderred a few self-inverse gates
bool is_inverse(const gate& g1, const gate& g2 )
{
    if( (is_S_gate( g1 ) && is_S_star_gate( g2 )) ||
       (is_S_gate( g2 ) && is_S_star_gate( g1 )) ||
       (is_T_gate( g1 ) && is_T_star_gate( g2 )) ||
       (is_T_gate( g2 ) && is_T_star_gate( g1 )))
    {
        if( g1.targets().front() == g2.targets().front())
        {
            return true;
        }
    }
    if( !same_type( g1, g2 )){
        return false;
    }
    if( !(is_toffoli( g1 ) || is_hadamard( g1 ) ) )
    {
        return false;
    }
    if(( g1.targets().size() != g2.targets().size() ) ||
       ( g1.controls().size() != g2.controls().size() ))
    {
        return false;
    }
    if( ( g1.targets().size() != 1 ) ||
       ( g1.targets().front() != g2.targets().front()) )
    {
        return false;
    }
    if( ( g1.controls().size() == 1 ) &&
       ( g1.controls().front().line() != g2.controls().front().line() ) )
    {
        return false;
    }
    
    return true;
}
 
// check if the two gates may be inter changed
// WARNING: only written for Clifford+T gates
bool gates_can_move( const gate& g1, const gate& g2 )
{
    unsigned target_g2 = g2.targets().front();
    unsigned target_g1 = g1.targets().front();
    // only move hadamard if it does not intersect with control nor target
    if ( is_hadamard( g1 ) )
    {
        if ( target_g2 == target_g1 ){
            return false;
        }
        if ( g2.controls().empty() )
        {
            return true;
        }
        else{
            unsigned control_g2 = g2.controls().front().line();
            if ( g1.targets().front() == control_g2 )
            {
                return false;
            }
        }
        return true;
    }
    // g1 is a NOT gate
    else if ( is_toffoli( g1 ) && g1.controls().empty() )
    {
        if ( is_toffoli( g2 ) )
        {
            // g2 is a CNOT
            if( !g2.controls().empty() )
            {
                return g2.controls().front().line() != target_g1;
            }
            return true;
        }
        assert ( g2.controls().empty() ); // only controlled gates are CNOTs
        return ( target_g2 != target_g1 );
    }
    // g1 is CNOT gate
    else if ( is_toffoli( g1 ) )
    {
        if ( !is_toffoli( g2 ) )
        {
            return ( g1.controls().front().line() != target_g2 ) && (target_g1 != target_g2 );
        }
        else if ( g2.controls().empty() )
        {
            return g1.controls().front().line() != target_g2;
        }
        else
        {
            return (g2.controls().front().line() != target_g1) &&
            (g1.controls().front().line() != target_g2);
        }
    }
    // g2 CNOT; g1 is not a Toffoli
    else if ( is_toffoli( g2 ) && !g2.controls().empty())
    {
        return ( target_g1 != g2.controls().front().line() ) && (target_g1 != target_g2 );
    }
    if( target_g1 != target_g2 )
    {
        return true;
    }
    // they have the same target
    // both gates must be one of S, S*, T, T*, or Z
    if( ( is_S_gate( g1 ) ||
         is_S_star_gate( g1 ) ||
         is_T_gate( g1 ) ||
         is_T_star_gate( g1 ) ||
         is_Z_gate( g1 )
       ) && (
             is_S_gate( g2 ) ||
             is_S_star_gate( g2 ) ||
             is_T_gate( g2 ) ||
             is_T_star_gate( g2 ) ||
             is_Z_gate( g2 )
       ))
    {
        return true;
    }
    return false;
}

// no controls or targets intersect
bool gates_do_not_intersect( const gate& g1, const gate& g2 )
{
    unsigned target_g2 = g2.targets().front();
    unsigned target_g1 = g1.targets().front();
    if ( !g1.controls().empty() && ( target_g2 == g1.controls().front().line() ) )
    {
        return false;
    }
    if ( !g2.controls().empty() && ( target_g1 == g2.controls().front().line() ) )
    {
        return false;
    }
    if ( !g1.controls().empty() && !g2.controls().empty() &&
        g1.controls().front().line() == g2.controls().front().line() )
    {
        return false;
    }
    return target_g1 != target_g2;
}

/* Check if both are T gates or S. Merge them if possible
 */
bool gates_can_merge( const gate& g1, const gate& g2, gate& res)
{
    if ( g1.targets().front() == g2.targets().front() )
    {
        if ( is_T_gate( g1 ) && ( is_T_gate( g2 ) ) )
        {
            res = g1;
            res.set_type( pauli_tag( pauli_axis::Z, 2u, false ) );
            return true;
        }
       if ( is_T_star_gate( g1 ) && ( is_T_star_gate( g2 ) ) )
            {
                res = g1;
                res.set_type( pauli_tag( pauli_axis::Z, 2u, true ) );
                return true;
            }
        if ( is_S_gate( g1 ) && ( is_S_gate( g2 ) ) )
        {
            res = g1;
            res.set_type( pauli_tag( pauli_axis::Z, 1u, false ) );
            return true;
        }
        if ( is_S_star_gate( g1 ) && ( is_S_star_gate( g2 ) ) )
        {
            res = g1;
            res.set_type( pauli_tag( pauli_axis::Z, 1u, false ) );
            return true;
        }

    }
    return false;
}
   
bool is_T_gate( const gate& g )
{
    if ( is_pauli( g ) )
    {
        const auto& tag = boost::any_cast<pauli_tag>( g.type() );
        return ( ( tag.axis == pauli_axis::Z ) &&
                ( tag.root == 4u ) &&
                !tag.adjoint );
    }
    return false;
}
    
bool is_T_star_gate( const gate& g )
{
    if ( is_pauli( g ) )
    {
        const auto& tag = boost::any_cast<pauli_tag>( g.type() );
        return ( ( tag.axis == pauli_axis::Z ) &&
                ( tag.root == 4u ) &&
                tag.adjoint );
    }
    return false;
}

bool is_S_gate( const gate& g )
{
    if ( is_pauli( g ) )
    {
        const auto& tag = boost::any_cast<pauli_tag>( g.type() );
        return ( ( tag.axis == pauli_axis::Z ) &&
                ( tag.root == 2u ) &&
                !tag.adjoint );
    }
    return false;
}
    
bool is_S_star_gate( const gate& g )
{
    if ( is_pauli( g ) )
    {
        const auto& tag = boost::any_cast<pauli_tag>( g.type() );
        return ( ( tag.axis == pauli_axis::Z ) &&
                ( tag.root == 2u ) &&
                tag.adjoint );
    }
    return false;
}

bool is_Z_gate( const gate& g )
{
    if ( is_pauli( g ) )
    {
        const auto& tag = boost::any_cast<pauli_tag>( g.type() );
        return ( ( tag.axis == pauli_axis::Z ) &&
                ( tag.root == 1u ));
    }
    return false;
}
    
bool is_RZ_gate( const gate& g ){
    if ( is_rotation( g ) )
    {
        const auto& tag = boost::any_cast<rotation_tag>( g.type() );
        return ( ( tag.axis == rotation_axis::Z ) );
    }
    return false;
}
}



// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
