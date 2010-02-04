
/*
 * Copyright (C) 2009 Kirtika B Ruchandani <kirtibr@gmail.com> 
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


/**
 * @file
 * Single function module to do good simulation
 */

#include <iostream>
#include "simulate.h"
using namespace std;


/**
 * @param
 * Reference to the circuit object to be simulated
 *
 * This function is a mere wrapper over Evaluate
 * in evaluate.cpp. It reads the inputs from a file
 * containing one value per line, in the order in which
 * the PriInputs field stores wires, and sets those values
 * and calls Evaluate, printing the outputs at the end. 
 */


bool Simulate_Good(Circuit& circuit)
{
    ifstream inputFile;
    ofstream outputFile;
    string inName;
    string outName;
    int inputval;

    list<Element*>::iterator innerIter;

    cout << "Starting simulation" << endl;
    cout << "Please enter the name of the file with input vectors " << endl;
    
    /// Input vectors can be only 0,15,5 standing for 0,1,unknown
    cin >> inName;
    inputFile.open(inName.c_str(),ios::in);
    

    cout << "Reading input values from file " << inName <<  endl;
        
    map<string,Wire*> ::iterator iter =  (circuit.PriInputs).begin();
    for (;iter != (circuit.PriInputs).end(); iter++)
    {   
        if ( inputFile >> inputval && (inputval == 0 || inputval == 15 || inputval == 5) )
        {
            iter->second->value = (Value) inputval;
            cout << "Setting  value of " <<  iter->second->id << " to  "  << inputval <<  endl;   
            
            /// If the PI has fanout, then all branches should also be set to the
            /// same value
            if (iter->second->outputs.size() > 1 )
            {
                innerIter = iter->second->outputs.begin();
                for ( ;innerIter != iter->second->outputs.end();innerIter++)
                {

                    cout << "Setting  value of branch " <<  (*innerIter)->id  << " of  stem "<<  iter->second->id << " to  "  << inputval <<  endl;   
                   ((Wire*) (*innerIter))->value = (Value) inputval; 
                }


            }


        }
        else 
        {
            cout << "Unacceptable  value of " <<  iter->second->id << " is  "  << inputval <<  endl;   
            cout << "Exiting" << endl;
            cout << "Unacceptable  value of " <<  iter->second->id << " is  "  << inputval <<  endl;   
            exit(0);
        }
    }

    cout << "Accepted all inputs, now calling circuit evaluate" << endl;
    cout << endl << endl;
    
    Evaluate(circuit);
    cout << "Evaluation done. Now printing outputs" << endl;
    cout << "Please enter the name of the file to print output vectors " << endl;
    
    cin >> outName;
    outputFile.open(outName.c_str(),ios::out);
    
    
    iter =  (circuit.PriOutputs).begin();
    for (;iter != (circuit.PriOutputs).end(); iter++)
    {
       outputFile << iter->second->value << endl;   
    }


    cout << endl << endl;
    
    outputFile.close();
    inputFile.close();

    return true;
}
