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

/**
 * @file tabu.hpp
 *
 * @brief Tabu Search to optimize reversible circuit
 *
 * @author A.G.A.
 * @2
 */

#ifndef CLI_TABU_COMMAND_HPP
#define CLI_TABU_COMMAND_HPP

#include <reversible/circuit.hpp>
#include <cli/cirkit_command.hpp>

namespace cirkit
{

class tabu_command : public cirkit_command
    {
    public:
        tabu_command( const environment::ptr& env );

protected:
  bool execute();
  rules_t validity_rules() const;

private:
  unsigned int iteration = 500u;
	unsigned int opt = 0u;
	unsigned int penalization = 10u;
	unsigned int neighborhood = 10u;
	unsigned int overlap = 70u;
	
public:
  log_opt_t log() const;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
