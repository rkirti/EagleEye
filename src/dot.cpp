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
/*
static void NodePrint (Node* child, Node* parent, FILE* file, int parent_i, int *i) 
{
    int cur_i = (*i)++;

    assert (parent == child->p);
    if (!child->lcp)
        fprintf (file, "node [label=\"%d\" shape = circle fillcolor=\"#44AA44\"]; N%d;\n", child->key, cur_i);
    else 
        fprintf (file, "node [label=\"%d:%d:%d\" shape = circle fillcolor=\"#44AA44\"]; N%d;\n", child->max1, child->max2, child->max3, cur_i);
    fprintf (file, "N%d -> N%d;\n", parent_i, cur_i);
    if (child->lcp) {
        // Print pointer from parent
        NodePrint (child->lcp, child, file, cur_i, i); 
    }
    if (child->ccp) {
        // Print pointer from parent
        NodePrint (child->ccp, child, file, cur_i, i);
    }
    if (child->rcp) {
        // Print pointer from parent 
        NodePrint (child->rcp, child, file, cur_i, i);
    }
}
*/

void CircuitGraphPrint () 
{
    Wire* curWire;
    Gate* curGate;
    int level = 0;
    map<string,Wire*>::iterator iter;
    list<Element*>::iterator innerIter;
    multimap<int,Element*>:: iterator levelIter;


    ofstream graphFile;

    cout << endl;
    
    
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
   
    // Printing the primary input nodes
    iter =  (circuit.PriInputs).begin();
    for (;iter != (circuit.PriInputs).end(); iter++)
    {
        curWire = iter->second;
        innerIter = curWire->outputs.begin();
        for ( ;innerIter != curWire->outputs.end();innerIter++)
            graphFile <<  (curWire->id) << "->" <<  ((*innerIter)->id) << ";" << endl;
           
    }

    graphFile.close();
}





void DefineGraphNodes(FILE* file)
{
    map<string,Wire*>::iterator iter;
    Wire* curWire;

    //Define all wire nodes
    iter =  (circuit.Netlist).begin();

    for (;iter != (circuit.PriInputs).end(); iter++)
    {
        curWire = iter->second;
        const char* name = (curWire->id).c_str();
        fprintf (file, "node [label = \"%s\n%d\" shape = circle fillcolor=\"#44AA44\"];\n",name,curWire->value);
    }
}

/*

    file = fopen (filename, "w");

    fprintf (file, "digraph %s { \n",circuitname);
    fprintf (file, "rankdir = LR;\n");
    fprintf (file, "node [label=\"\" style=filled fixedsize=true width=1.6 height=1.6];\n");



    if (!root->lcp)
        fprintf (file, "node [label=\"%d\" shape = circle fillcolor=\"#44AA44\"]; N%d;\n", root->key, i++);
    else 
        fprintf (file, "node [label=\"%d:%d:%d\" shape = circle fillcolor=\"#44AA44\"]; N%d;\n", root->max1, root->max2, root->max3, i++);

    // Start traversal
    if (root->lcp) {
        NodePrint (root->lcp, root, file, 0, &i); 
    }
    if (root->ccp) {
        NodePrint (root->ccp, root, file, 0, &i);
    }
    if (root->rcp) {
        NodePrint (root->rcp, root, file, 0, &i);
    }

    fprintf (file, "} \n");
    fclose (file);
}
*/