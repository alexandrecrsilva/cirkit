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

#include <iostream>
#include <cmath>
#include "trans_path.hpp"
#include "move_qubit.hpp"

namespace cirkit
{
   
    void TransPath::add( MoveQubit q ){
        tpath.push_back( q );
    }
    void TransPath::print(){
        for ( auto &p : tpath ) {
            p.print();
        }
        std::cout << "cost = " << cost() << std::endl;
    }

    // Print to a file
    void TransPath::print( std::ofstream& graphfile ){
        for ( auto &p : tpath ) {
            p.print( graphfile );
        }
        graphfile << "cost = " << cost() << std::endl;
    }

    // Check if the movement has a cnot3 and update it
    void TransPath::movCnot3(){
        if(tpath.size() > 1)
        {
            if(tpath.size() == 2)
            {
                if(tpath[0].getType() == cab && tpath[1].getType() == nop) // two consecutive arrows
                {
                    unsigned a = tpath[tpath.size()-2].getA();  // get the parameters for the cnot3
                    unsigned b = tpath[tpath.size()-1].getA();  //
                    unsigned c = tpath[tpath.size()-1].getB();  //
                    tpath.pop_back();                               // remove the cab and 
                    tpath.pop_back();                               // the nop movements
                    tpath.push_back( MoveQubit( cnot3, a, b, c ) ); // append the cnot3
                }
            }
            else
            {
                if(tpath[tpath.size()-1].getType() == nop)  // cnot3 movement must end with nop
                {
                    unsigned movement = 0;  // count the number of cab (future cnot3)  
                    for (int i = tpath.size()-2 ; i >= 0 ; --i)
                    {
                        if(tpath[i].getType() != cab ) // must be consecutive cab movement
                            break;
                        else
                            ++movement;
                    }
                    if(movement >= 2)
                    {
                        std::vector<MoveQubit> cnot3_list; // vector to create the cnot3
                        unsigned a,b,c;
                        for (int i = 0; i < movement; ++i)
                        {
                            a = tpath[tpath.size()-2].getA(); // 
                            b = tpath[tpath.size()-1].getA(); // get the data for the cnot3
                            c = tpath[tpath.size()-1].getB(); //
                            cnot3_list.insert( cnot3_list.begin(), MoveQubit( cnot3, a, b, c ) );
                            tpath.pop_back();
                        }

                        tpath.pop_back();
                        for ( auto &p : cnot3_list ) // append the cnot3 in the transformation
                            tpath.push_back( p );
                    }       
                }
                
            }    
        }
    }

    // Possible optimization in the transformations - two adjacents hadamards
    int TransPath::opt(){
        int total = 0;
        for (int i = 0; i < tpath.size()/2; i=i+2){
            if( tpath[i].getType() == cba && tpath[i+1].getType() == cab) 
                total += 4;
            else if( tpath[i].getType()== cba && tpath[i+1].getType() == tba) 
                total += 4;
            else if( tpath[i].getType() == cba && tpath[i+1].getType() == flip)
                total += 4;
            else if( tpath[i].getType() == tab && tpath[i+1].getType() == cab)
                total += 4;
            else if( tpath[i].getType() == tab && tpath[i+1].getType() == tba)
                total += 4;
            else if( tpath[i].getType() == tab && tpath[i+1].getType() == flip)
                total += 4;
        }
        return total;
    }

    // Calculate the cost of the cnot3 movement
    int TransPath::cnot3Cost(){
        int res = 0;
        for ( auto &p : tpath ) {
            if(p.getType() == cnot3)
                ++res;
        }
        if(res > 0) // Formula: 2^n + 2^(n+1) - 2
            return (pow(2,res)+pow(2,++res)-2);
        else
            return 1;
    }

    int TransPath::cost(){
        int res = 0;
        for ( auto &p : tpath ) {
            res += p.cost();
        }
        return res + ( cnot3Cost() - 1 ); 
    }
    
    // add the cost of the inverse path
    int TransPath::costPlus(){
        int res = 0;
        for ( auto &p : tpath ) {
            res += p.cost();
        }
        return 2*res - tpath.back().cost() + ( cnot3Cost() - 1 );
    }
    
    void TransPath::addInverse(){
        MoveQubit q;
        for(int i = tpath.size() - 2; i >= 0; i-- )
        {
            if(tpath[i].getType() != cnot3) // cnot3 has no inverse
            {
                q = tpath[i];
                q.invert();
                tpath.push_back( q );    
            }
        }
    }
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
