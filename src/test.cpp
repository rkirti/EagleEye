
/*
 * $Id$
 * Copyright (C) 2009 Kirtika B Ruchandani <kirtibr@gmail.com> 
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include <iostream>
using namespace std;
#include "circuit.h"


/**
 * Print out the set of test vectors recorded by a particular
 * test object after a successful run.
 */ 
void Test::Print_Test_Set()
{
    list<TestVector>::iterator current = tests.begin();
    for (;current != tests.end(); current++)   
    {

        vector<Value>::iterator input;
        for (input=(*current).begin(); input != (*current).end(); input++)
        {
            switch (*input)
            {


                case ZERO:
                    cout << "0";
                    break;
                case ONE:
                    cout << "1";
                    break;
                case U:
                    cout << "U";
                    break;
                case D:
                    cout << "D";
                    break;
                case DBAR:
                    cout << "DBAR";
                    break;

            }

        }

            
        cout << endl;


    }

    return;
}


/// Record the outputs, either for later
/// comparison with good simulation or to print at which output  
/// the fault is detected.
vector<Value> CaptureOutput(Circuit& circuit)
{
    vector<Value> output;
    map<string,Wire *>::iterator iter = circuit.PriOutputs.begin();
    for (;iter != circuit.PriOutputs.end(); iter++)
        output.push_back((iter->second)->value);
    return output;
}

/// Record the input values as soon as a fault is detected
/// successfully at the primary outputs and put it in the 
/// test vector list.
vector<Value> CaptureInput(Circuit& circuit)
{
    vector<Value> output;
    map<string,Wire *>::iterator iter = circuit.PriInputs.begin();
    for (;iter != circuit.PriInputs.end(); iter++)
        output.push_back((iter->second)->value);
    return output;
}






