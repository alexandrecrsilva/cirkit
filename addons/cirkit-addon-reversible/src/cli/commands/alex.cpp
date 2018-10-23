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

#include "alex.hpp"

#include <cmath>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <iostream>

#include <boost/program_options.hpp>

#include <xtensor/xio.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor-blas/xlinalg.hpp>

#include <cli/reversible_stores.hpp>
#include <reversible/utils/matrix_utils.hpp>
#include <alice/rules.hpp>


namespace cirkit
{

using boost::program_options::value;

alex_command::alex_command( const environment::ptr& env )
	: cirkit_command( env, "Alex test" )
{
	opts.add_options()
	( "input,i",    value( &input ),    "print single input -- use it in binary" )
	;

}

command::rules_t alex_command::validity_rules() const
{
  return {has_store_element<circuit>( env )};
}


void print_all_matrix(xt::xarray<complex_t>& t, unsigned int& q)
{
	for (int i = 0; i < t.size(); ++i)
	{
		if(i % q == 0)
			std::cout << std::endl;
		std::cout << " " << t[i];
	}
	std::cout << std::endl;
}

void print_line(xt::xarray<complex_t>& t, unsigned int& q, std::string input)
{
	unsigned int j = 0;
	bool first = true;
	std::string p;
	unsigned int k = log10(q)/log10(2);

	unsigned int line;
	line = std::stoi(input, nullptr, 2);
	if(line >= q)
	{
		std::cout << "Out of range. The circuit has " << k << " qubits." << std::endl;
	}
	else
	{
		for (int i = line*q; i < (line+1)*q; ++i, ++j)
		{
			if(t[i] != 0.0)
			{
				if(first)
				{
					p = std::bitset<8>(j).to_string();
					// std::cout << std::endl;
					std::string pp = std::bitset<8>(line).to_string();
					std::cout << "input: " << pp.substr(pp.length() - k) << " => " << t[i] << " " << p.substr(p.length() - k);
					first = false;
				}
				else
				{
					p = std::bitset<8>(j).to_string();
					std::cout << " + " << t[i] << " " << p.substr(p.length() - k);
				}
			}
		}
		std::cout << std::endl;
	}
}

void print_solution(xt::xarray<complex_t>& t, const unsigned int& q)
{
	unsigned int j = 0;
	bool first = true;
	std::string p;
	unsigned int k = log10(q)/log10(2);
	for (int i = 0; i < t.size(); ++i, ++j)
	{
		if(i % q == 0)
		{
			p = std::bitset<8>(i/q).to_string();
			std::cout << std::endl;
			std::cout << "input: " <<  p.substr(p.length() - k) << " => ";
			j = 0;
			first = true;
		}
		if(t[i] != 0.0)
		{
			if(first)
			{
				p = std::bitset<8>(j).to_string();
				std::cout << " " << t[i] << " " << p.substr(p.length() - k);
				first = false;
			}
			else
			{
				p = std::bitset<8>(j).to_string();
				std::cout << " + " << t[i] << " " << p.substr(p.length() - k);
			}
		}
	}
	std::cout << std::endl;

  // std::cout << t << std::endl;
}

int get_max_element(std::vector<std::vector<unsigned>>& m, unsigned& l, unsigned& c)
{
    unsigned h = 0;
    std::cout << "AAA " << m.size() << std::endl;

    for (int i = 0; i < m.size(); ++i)
    {
        for (int j = 0; j < m[i].size(); ++j)
        {
            if(i != j && m[i][j] > h)
            {
                h = m[i][j];
                l = i;
                c = j;
                std::cout << "AAA" << std::endl;
            }
        }
    }
    return h;
}

 using matrix = std::vector<std::vector<unsigned>>;


// return the number of different gates of the circuit
int getNumberDifGates( matrix& c )
{
	unsigned qtd = 0;
	for (int i = 0; i < c.size(); ++i)
		for (int j = 0; j < c[i].size(); ++j)
			if( c[i][j] > 0 )
				++qtd;
	return qtd;
}

// Function to print the objective function
void printObjectiveFunction( matrix& qx, unsigned dif_gates )
{
	std::cout << "min:\t";
	for (int i = 0; i < dif_gates; ++i)
	{
		for (int j = 0; j < 5; ++j)
		{
			for (int k = 0; k < 5; ++k)
			{
				if(i == dif_gates-1 && j == 4 && k == 3)
					std::cout << qx[j][k] << "G" << i << "c" << j << k << ";";
				else if( j != k )
					std::cout << qx[j][k] << "G" << i << "c" << j << k << " + ";
			}
		}
		std::cout << std::endl << "\t";
	}
}

// Create a matrix with 0's
void createMatrix( matrix& m, unsigned size )
{
	std::cout << "Creating matrix..." << std::endl;
  	std::vector<unsigned> v;
	for (int i = 0; i < size; ++i)
		v.push_back(0);
	for (int i = 0; i < size; ++i)
		m.push_back(v);
}

// Create a matrix with the cnots 
void generateMatrixCnots( circuit& circ, matrix& m )
{
	std::cout << "Generating matrix..." << std::endl;	
  	unsigned target, control;
	for ( const auto& gate : circ )
	{
		if( !gate.controls().empty() )
		{
		  target = gate.targets().front();
		  control = gate.controls().front().line();
		  ++m[control][target];
		}
	}
}

// Print the cnots in the circuit
void printMatrixCnots( matrix& m )
{
	std::cout << "Printing matrix..." << std::endl;
	for (int i = 0; i < m.size(); ++i)
	{
		for (int j = 0; j < m[i].size(); ++j)
			std::cout << "\t" << m[i][j];
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

// Control dependency
void writeDepControlControl( unsigned l, unsigned c0, unsigned c1, unsigned size )
{
	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
		{
			if(i != j)
			{
				std::cout << "G" << l << c0;
				std::cout << "c" << i << j;
				std::cout << " <= ";
				unsigned aux = 0;
				for (int k = 0; k < size; ++k)
				{
					if(k != j && k != i)
					{
						++aux;
						std::cout << "G" << l << c1;
						std::cout << "c" << i << k;
						if(aux == size-2)
							std::cout << ";";
						else	
							std::cout << " + ";	
					}
				}
				std::cout << std::endl;	
			}
		}
	}
}

// Target dependency
void writeDepTargetTarget( unsigned c, unsigned l0, unsigned l1, unsigned size )
{
	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
		{
			if(i != j)
			{
				std::cout << "G" << l0 << c;
				std::cout << "c" << j << i;
				std::cout << " <= ";
				unsigned aux = 0;
				for (int k = 0; k < size; ++k)
				{
					if(k != j && k != i)
					{
						++aux;
						std::cout << "G" << l1 << c;
						std::cout << "c" << k << i;
						if(aux == size-2)
							std::cout << ";";
						else	
							std::cout << " + ";	
					}
				}
				std::cout << std::endl;	
			}
		}
	}
}

// Control dependency
void writeDepControlTarget( unsigned c, unsigned l0, unsigned l1, unsigned size )
{
	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
		{
			if(i != j)
			{
				std::cout << "G" << l0 << c;
				std::cout << "c" << j << i;
				std::cout << " <= ";
				unsigned aux = 0;
				for (int k = 0; k < size; ++k)
				{
					if(k != j && k != i)
					{
						++aux;
						std::cout << "G" << l1 << c;
						std::cout << "c" << k << i;
						if(aux == size-2)
							std::cout << ";";
						else	
							std::cout << " + ";	
					}
				}
				std::cout << std::endl;	
			}
		}
	}
}

void searchDepControl( matrix& m, unsigned l, unsigned c )
{
	
	for (int i = 0; i < m[l].size(); ++i)
	{
		for (int j = i+1; j < m[l].size(); ++j)
		{
			
		}
	}
}

bool alex_command::execute()
{
	circuit circ = env->store<circuit>().current();
	matrix output;
	matrix qx4 ={{0,4,4,10,10},
			  {0,0,4,10,10},
			  {0,0,0,4,0},
			  {3,3,0,0,0},
			  {10,10,4,4,0}};

  	createMatrix( output, circ.lines() );
  	generateMatrixCnots( circ, output );
	printMatrixCnots( output );
	// writeDepControl( 0, 1, 2, 5 );
	writeDepTarget( 1, 2, 3, 5 );
  
	return true;
}

command::log_opt_t alex_command::log() const
{
  return log_opt_t({
	  {"runtime",       statistics->get<double>( "runtime" )}
	});
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
