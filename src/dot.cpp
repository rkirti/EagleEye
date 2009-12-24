/*
 * Code to get a visual representation of the circuit
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

#include "circuit.h"
#include <iostream>
#include <fstream>
#include "dot.h"
using namespace std;


void CircuitGraphPrint () 
{
    Wire* curWire;
    Gate* curGate;
    int level = 0;
    map<string,Wire*>::iterator iter;
    list<Element*>::iterator innerIter;
    multimap<int,Element*>:: iterator levelIter;

    ofstream graphFile;
    graphFile.open("trial.dot");
   

    // Print .dot header
    graphFile << "digraph ckt { \n";
    graphFile <<  "rankdir = LR;\n";

    //Set the common properties for each node
    graphFile << "node [label=\"\" style=filled fixedsize=true width=1.6 height=1.6];\n";

    // define all the wire nodes
    iter =  (circuit.Netlist).begin();
    for (;iter != (circuit.Netlist).end(); iter++)
    {
        curWire = iter->second;
        graphFile << "node [label = " << curWire->id << " " << curWire->value <<  "  shape = circle ];  " << curWire->id <<  ";\n";
    }


    
    // define all the gate nodes
    do
    {
        for (levelIter = ((circuit.Levels).equal_range(level)).first; 
                levelIter !=((circuit.Levels).equal_range(level)).second; levelIter++)
        {
            if ( (levelIter->second)->type == GATE)
            {
                curGate =  dynamic_cast<Gate*> (levelIter->second);
                string name = (curGate->id);

                graphFile << "node [label = " << name <<  " shape = record ];  " << curGate->id <<  "  ;\n";
            }
        }
    }while ((circuit.Levels).find(++level) != (circuit.Levels).end() ); 


    
    // Logic: For every wire in the circuit, print its output
    // If the wire's input is a Gate, then print that connection
    // i.e gate going to that wire
    iter =  (circuit.Netlist).begin();
    for (;iter != (circuit.Netlist).end(); iter++)
    {
        curWire = iter->second;
       
        // First print the input if needed
        if ( curWire->input && curWire->input->type == GATE)
            graphFile <<  (curWire->input->id) << "->" <<  ( curWire->id) << ";" << endl;


        // Take care of outputs now
        innerIter = curWire->outputs.begin();
        for ( ;innerIter != curWire->outputs.end();innerIter++)
            graphFile <<  (curWire->id) << "->" <<  ((*innerIter)->id) << ";" << endl;
           
    }
 
    // End the graph printing routine 
    graphFile << " \n } \n";
    graphFile.close();
}
