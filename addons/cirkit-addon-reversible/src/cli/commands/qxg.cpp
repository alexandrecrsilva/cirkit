/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "qxg.hpp"

#include <fstream>
#include <algorithm>

#include <alice/rules.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/program_options.hpp>
#include <reversible/circuit.hpp>
#include <reversible/gate.hpp>
#include <cli/reversible_stores.hpp>
#include <reversible/pauli_tags.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/io/write_qc.hpp>
#include <reversible/io/print_circuit.hpp>

//Matrix with the cost of each possible cnot
int static const map[5][5] = {{0,0,0,10,10}, {4,0,0,10,10}, {4,4,0,4,4}, {10,10,0,0,0}, {10,10,0,4,0}};

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

qxg_command::qxg_command( const environment::ptr& env )
    : cirkit_command( env, "IBM QX mapping naive algorithm (only QX2 for now)" )
{
    opts.add_options()
 
    ;
  add_new_option();
}


command::rules_t qxg_command::validity_rules() const
{
  return {has_store_element<circuit>( env )};
}

//Change two lines of the circuit
void manipulate_matrix( int(& m1)[5][5], int x, int y, int(& m2)[5][5], int(& p)[5] )
{
    int aux;

    aux = p[x];
    p[x] = p[y];
    p[y] = aux;
    for(int j=0; j<5; ++j)
    {
        aux = m1[x][j];
        m1[x][j] = m1[y][j];
        m1[y][j] = aux;
        m2[x][j] = m1[x][j] * map[x][j]; 
        m2[y][j] = m1[y][j] * map[y][j];
    }
    for(int i=0; i<5; ++i)
    {
        aux = m1[i][x];
        m1[i][x] = m1[i][y];
        m1[i][y] = aux;
        m2[i][x] = m1[i][x] * map[i][x]; 
        m2[i][y] = m1[i][y] * map[i][y];
    }
}

void print_matrix( int m[5][5])
{
    for(int i=0; i<5; ++i)
    {
        for(int j=0; j<5; ++j)
        {
            std::cout << m[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

//Search rows for the higher target cost
int higher_target_cost(int m1[5][5], int m2[5][5])
{
    int cost, higher_cost = 0, index, higher_qtd_cnot = 0, qtd_cnot;
    for(int j=0; j<5; ++j)
    {
        cost = 0;
        qtd_cnot = 0;
        for(int i=0; i<5; ++i)
        {
            cost += m1[i][j];
            qtd_cnot += m2[i][j];
        }

        if(cost > higher_cost)
        {
            higher_cost = cost;
            higher_qtd_cnot = qtd_cnot;
            index = j;
        }
        else if(cost == higher_cost && qtd_cnot > higher_qtd_cnot)
        {
            higher_qtd_cnot = qtd_cnot;
            index = j;
        }
    }
    return index;
}

//Create cnots matrix and cost matrix
unsigned initial_matrix(circuit circ, int(& cnots)[5][5], int(& map_cost)[5][5])
{
    unsigned target, control;
    unsigned cost = 0;
    for ( const auto& gate : circ )
    {
        if ( is_toffoli( gate ) && !gate.controls().empty() )
        {   
            target = gate.targets().front();
            control = gate.controls().front().line();
            cost += map[control][target];
            ++cnots[control][target];
            map_cost[control][target] = cnots[control][target] * map[control][target];
        }
    }
    return cost;
}

//Update the circuit cost
unsigned matrix_cost(int m[5][5])
{
    unsigned cost = 0;
    for(int i=0; i<5; ++i)
        for(int j=0; j<5; ++j)
            cost += m[i][j];
    return cost;
}

//Search for the column in the matrix with the lowest cost to swap
int find_column(int cnots[5][5], int h)
{
    int cost, lower, column;
    bool first = true;
    for(int j=0; j<5; j++)
    {
        if( j != h )
        {
            cost = 0;
            for(int i=0; i<5; i++)
            {
                cost += cnots[i][h] * map[i][j]; 
            }
            if(first)
            {
                lower = cost;
                first = false;
            }
            if( cost < lower )
            {
                lower = cost;
                column = j;
            }
        }
    }
    return column;
}

void print_permutation(int perm[5])
{
    for(int i=0; i<5; i++)
        std::cout << " " << perm[i];    
    std::cout << std::endl;
}

bool qxg_command::execute()
{
    auto& circuits = env->store<circuit>();
    circuit circ = circuits.current();
    unsigned cost, lower_cost;
    int cnots[5][5] = {0};
    int map_cost[5][5] = {0};
    int h, c;
    int perm[5] = {0, 1, 2, 3, 4};
    int best_perm[5] = {0, 1, 2, 3, 4};

    cost = initial_matrix(circ, cnots, map_cost);
    cost = cost + circ.num_gates();
    //std::cout << circ.num_gates() << std::endl;
    std::cout << "initial cost: " << cost << std::endl;
    lower_cost = cost;
    //std::cout << "cnots matrix: " << std::endl;
    //print_matrix(cnots);
    //std::cout << "cost matrix: " << std::endl;
    //print_matrix(map_cost);
    int it;
    do
    {
        //std::cout << "Perm: ";
        //print_permutation(perm);
        h = higher_target_cost(map_cost, cnots);
        //std::cout << "column higher cost: " << h << std::endl;
        c = find_column(cnots, h);
        manipulate_matrix( cnots, h, c, map_cost, perm );
        it++;
        cost = matrix_cost(map_cost) + circ.num_gates();
        //std::cout << "total cost: " << cost << std::endl;
        if(cost < lower_cost)
        {
            lower_cost = cost;
            for(int i=0; i<5; i++)
                best_perm[i] = perm[i];
        }

    } while (it != 5); //five times was a test, because of the five qubits (it will change later)
    
    std::cout << "Best permutation found (cost " << lower_cost << "):";
    print_permutation(best_perm);
    
    //This print the permutation like the ibm command
    std::cout << "ibm representation: ";
    for(int i=0; i<5; i++)
    {
        for(int j=0; j<5; j++)
        {
            if(best_perm[j] == i)
            {
                std::cout << " " << j;
                break;
            }
        }
    }
    std::cout << std::endl;  
    
    return true;
}

command::log_opt_t qxg_command::log() const
{
  return log_opt_t({{"runtime", statistics->get<double>( "runtime" )}});
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
