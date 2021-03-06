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

#include "move_qubit.hpp"
#include <iostream>

namespace cirkit
{
    const std::string MoveQubit::type_name[11] = { "cab", "cba", "tab", "tba", "cabi", "cbai", "tabi", "tbai", "nop", "flip", "cnot3"};
    const int MoveQubit::move_cost[11] = {6, 5, 5, 6, 6, 5, 5, 6, 0, 4, 0}; // here the cost of cnot3 is zero due the value is not constant
    MoveQubit::MoveQubit( move_qubit_type t, int a, int b){
        mv_type = t;
        v = a;
        w = b;
    }
    // Only for the cnot3 for now
    MoveQubit::MoveQubit( move_qubit_type t, int a, int b, int c){
        mv_type = t;
        v = a;
        w = b;
        z = c;
    }
    
    void MoveQubit::set( move_qubit_type t, int a, int b){
        mv_type = t;
        v = a;
        w = b;
    }
    
    void MoveQubit::print(){
        if(z != -1) // default z is -1. It means only cnot3 has z different of -1
            std::cout << type_name[mv_type] << " " << v << " " << w << " " << z << "; ";
        else
            std::cout << type_name[mv_type] << " " << v << " " << w << "; ";
    }

    // Print to a file
    void MoveQubit::print( std::ofstream& graphfile ){
        if(z != -1) // default z is -1. It means only cnot3 has z different of -1
            graphfile << type_name[mv_type] << " " << v << " " << w << " " << z << " ; ";
        else
            graphfile << type_name[mv_type] << " " << v << " " << w << " ; ";
    }

    int MoveQubit::cost(){
        return move_cost[mv_type];
    }
    
    inline move_qubit_type invert_type( move_qubit_type a){
        /*if( a <= tba ){
            return static_cast<move_qubit_type>( a + 4);
        }
        if( a <= tbai ){
            return static_cast<move_qubit_type>( a - 4);
        }
        return a;
         */
        return inverse_type[a];
    }
    
    void MoveQubit::invert(){
        mv_type = invert_type( mv_type );
    }
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
