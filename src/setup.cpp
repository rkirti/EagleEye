/*
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
#include "circuit.h"
using namespace std;

extern ofstream CIRCUIT_DFILE;

// NOTE:  We add only PIs, POs, and intermediate gates to the levels

bool Levelize(Circuit* circuitPtr)
{
    Circuit circuit = *circuitPtr;
    int level=0;
    map<string,Wire*>::iterator iter =  (circuit.PriInputs).begin();
    multimap<int,Element*>:: iterator levelIter;

    CIRCUIT_DFILE << "Starting Levelize" << endl;

    // Adding the primary inputs to level zero
    CIRCUIT_DFILE <<  "Adding all primary inputs" << endl;
    
    for (;iter != (circuit.PriInputs).end(); iter++)
    {   
        CIRCUIT_DFILE << "Adding  " << iter->second->id << " at level 0 " << endl;
        (circuit.Levels).insert(pair<int,Element*>(0,(Element*)iter->second));
    }

    do
    {
        for (levelIter = ((circuit.Levels).equal_range(level)).first; 
                levelIter !=((circuit.Levels).equal_range(level)).second; levelIter++)
        {
            if ( (levelIter->second)->type == GATE)
            {
                Gate* curGate = dynamic_cast<Gate*>(levelIter->second);
                Wire* curWire = curGate->output;
                
                list<Element*>::iterator iter = (curWire->outputs).begin();
                for (;iter != (curWire->outputs).end(); iter++)
                {
                    Gate*  curGate = dynamic_cast<Gate*>(*iter);
                    if (!curGate) continue; 
                    curGate->tempInputs--;
                    if (curGate->tempInputs == 0) 
                    {

                        CIRCUIT_DFILE << "Adding  " << curGate->id << " at level  " << level+1 << endl;
                        (circuit.Levels).insert(pair<int,Element*>(level+1,(Element*)curGate));
                    }
                }
            }
            else if  ( (dynamic_cast<Wire*>(levelIter->second))->wtype == PI )
            {
                Wire* curWire =   dynamic_cast<Wire*>(levelIter->second);
                list<Element*>::iterator iter = (curWire->outputs).begin();
                for (;iter != (curWire->outputs).end(); iter++)
                {
                    Gate*  curGate = dynamic_cast<Gate*>(*iter);
                    if (!curGate) continue; 
                    curGate->tempInputs--;
                    if (curGate->tempInputs == 0) 
                    {

                        CIRCUIT_DFILE << "Adding  " << curGate->id << " at level  " << level+1 << endl;
                        (circuit.Levels).insert(pair<int,Element*>(level+1,(Element*)curGate));
                    }
                }

            } 

            else cout << "something wrong" << endl;


        }
    }while (   (circuit.Levels).find(++level) != (circuit.Levels).end() ); 
    CIRCUIT_DFILE << "Levelization completed" << endl << endl;
}





bool Resolve_Branches(Circuit* circuitPtr)
{
   Circuit circuit = *circuitPtr;
   map<string,Wire*>:: iterator iter = (circuit.Netlist).begin();
   CIRCUIT_DFILE << "Resolve Branches started" << endl;
   while (iter != (circuit.Netlist).end())
    {
        Wire* iwire = iter->second;
        
        // Condition for stemout
        // 1. If the wire is not a PO, output size is > 1
        // 2. If the wire is a PO output size is > 0
        if (  ( (int)(iwire->outputs).size() > 1 
            || ( (int) (iwire->outputs).size() > 0  && (iwire->wtype == PO))) 
            && (Wire_Not_Derived(iwire)))
        {

                CIRCUIT_DFILE << "Calling resolve wire for  " << iwire->id << endl;
                Resolve_Wire(circuitPtr,iwire);
        }
        else 
        {
            CIRCUIT_DFILE << "No need to resolve wire " << iwire->id << endl;
        }
        iter++;
    }
    CIRCUIT_DFILE << "Resolve branches completed successfully" << endl << endl;
    return true;
}



void Resolve_Wire(Circuit* circuitPtr,Wire* wire)
{

    Circuit circuit = *circuitPtr;
    // Assuming that initially all outputs of
    // a wire are gates
    //Steps:
    // 1. Find out if a wire has more than one outputs
    // 2. Create a new wire - with input as the orig wire
    // and output as the gate
    // 3. Update the orig wire's outputs to reflect the wires rather than gates
    // 4. Update the output gates (orig gates) inputs showing the new wires
    // as inputs

    cout << "Need to resolve wire:   " << wire->id << endl;
    list<Element*>:: iterator iter = (wire->outputs).begin();
    while ( iter != (wire->outputs).end() )
    {
        Element* curEle = (*iter);
        if (curEle->type == GATE)
        {

        Gate* gate = dynamic_cast<Gate*>(*iter);
        assert(gate); // dynamic_cast must not fail
        string newname = (wire->id)+"_"+(gate->id);
        newname = Check_Name_Present(circuitPtr,newname);
        // step 2. Add a new wire for this instance
        cout << "Adding derived wire:    " <<  newname  << endl;
        circuit.AddWire(newname.c_str(),CONNECTION);

        // The new wire's output should be the 
        // gate
        circuit.Add_Gate_To_Wire_Output(gate,newname.c_str());

        // The new wire's input should be the old wire and
        // the old wire' output should change from gate to 
        // new wire

        Wire* newwire = ((circuit.Netlist).find(newname))->second;
        newwire->input = wire;
         // finally update the gate's inputlist
        (gate->inputs).remove(wire);
        (gate->inputs).push_back(newwire);
        
        (*iter) = newwire;

       iter++;
        }


        else continue;
    }

    if (wire->wtype == PO)
    {
        cout << "***************PO with fanout detected*************" <<  endl;
        assert(false);
    }
}




bool Wire_Not_Derived(Wire* wire)
{
    if (wire->id.find("_") != string::npos ) return false;
    else return true;
}



string Check_Name_Present(Circuit* circuitPtr,string givenname)
{
    Circuit circuit =  *circuitPtr;
    map<string,int>::iterator iter = (circuit.RepeatInputs).find(givenname);
    cout << givenname <<endl;
    if (iter != (circuit.RepeatInputs).end())
    {
            iter->second += 1;
            string newname;
            newname = givenname + "_" + intToString(iter->second);
            cout << "Returning name   " << newname << endl;
         //   cout << "int value is  "  << iter->second << endl;
            return newname;
    }
    else 
     {
         (circuit.RepeatInputs).insert(pair<string,int>(givenname,0));
         cout << "Adding the namemap" << givenname << endl;
        //    cout << "Returning name   " << givenname << endl;
         return  givenname;
     }

}


