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


bool  Add_Wire(Circuit& circuit,const char *inName,WireType type)
{
	Wire *iwire = new Wire(inName,type);
	circuit.Netlist.insert( pair<string,Wire *>(inName,iwire) );
    iwire->value = U;
	
    if (type == PI)
        circuit.PriInputs.insert( pair<string,Wire*>(inName,iwire) );
	else if (type == PO)
		circuit.PriOutputs.insert( pair<string,Wire*>(inName,iwire) );

	return true;
}



void Add_Gate_To_Wire_Output( Circuit& circuit,Gate* gate,const char* wirename)
{
    Wire* iwire = ((circuit.Netlist).find(wirename))->second;
    assert( (circuit.Netlist).find(wirename) != (circuit.Netlist).end());
 //   cout << "Added gate  "  << gate->id <<  "  as output of wire  " << iwire->id << endl;
    iwire->outputs.push_back((Element*)gate);    
    return;
}


void Add_Gate_To_Wire_Input(Circuit& circuit, Gate* gate,const char* wirename)
{
    Wire* iwire = ((circuit.Netlist).find(wirename))->second;
    assert( (circuit.Netlist).find(wirename) != (circuit.Netlist).end());
  //  cout << "Added gate  "  << gate->id <<  "  as input of wire  " << iwire->id << endl;
    iwire->input = (Element*)gate;    
    return;
}




/* By the time a gate is added, all the wires have been added and 
 * their types are known
 * */

bool Add_Gate(Circuit& circuit,GateType type, char *name,char* output,char **inputs,int numSignals)
{
    Gate *gate = new Gate(name,type);
    map<string,Wire *>::iterator iter;

    // Some consistency checks before we add
    // the wires to the gates inputs and output
    // 1. Output must be a valid wire
    iter = circuit.Netlist.find(output);
    if (iter  ==  circuit.Netlist.end()) 
    {
        cout  << "Gate's output name %s does not represent a valid wire " << output << endl;
        return false;
    }

    // 2. If output type is PO, it should be
    // there in PO list
    if ((iter->second)->wtype == PO)
    {
        iter = circuit.PriOutputs.find(output);
        if (iter  ==  circuit.PriOutputs.end()) 
        {
            cout << "Gate's output name %s is supposedly PO but not present in PO list " << output << endl;
            return false;
        }
    }

    // Now that we are convinced, add the wire as gate's output
    // and the gate as wire's input :P
    Add_Gate_To_Wire_Input(circuit, gate,((iter->second)->id).c_str());
    gate->output = iter->second;

    while (numSignals--)
    {


        // Some consistency checks before we add
        // the wires to the gates inputs and output
        // 1. Input must be a valid wire
        iter = circuit.Netlist.find(inputs[numSignals]);
        if (iter  ==  circuit.Netlist.end()) 
        {
            cout << "Gate's input name %s does not represent a valid wire" << inputs[numSignals] << endl;
            return false;
        }

        // 2. If input type is PI, it should be
        // there in PI list
        if ((iter->second)->wtype == PI)
        {
            iter = circuit.PriInputs.find(inputs[numSignals]);
            if (iter  ==  circuit.PriInputs.end()) 
            {
                cout << "Gate's input name %s is supposedly PI but not present in PI list" << inputs[numSignals] << endl;
                return false;
            }
        }

        // Now that we are convinced, add the wire as gate's input
        gate->inputs.push_back(iter->second);
        Add_Gate_To_Wire_Output(circuit,gate,((iter->second)->id).c_str());
    }

    // Finally we add the gate to the circuit list
    gate->tempInputs = (gate->inputs).size();
    circuit.Gates.insert( pair<string,Gate *>(name, gate) );

    return true;
}


// NOTE:  We add only PIs, POs, and intermediate gates to the levels
bool Levelize(Circuit& circuit)
{
    int level=0;
    map<string,Wire*>::iterator iter =  (circuit.PriInputs).begin();
    multimap<int,Element*>:: iterator levelIter;

    cout << "Starting Levelize" << endl;

    // Adding the primary inputs to level zero
    cout <<  "Adding all primary inputs" << endl;
    
    for (;iter != (circuit.PriInputs).end(); iter++)
    {   
        cout << "Adding  " << iter->second->id << " at level 0 " << endl;
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

                        cout << "Adding  " << curGate->id << " at level  " << level+1 << endl;
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

                        cout << "Adding  " << curGate->id << " at level  " << level+1 << endl;
                        (circuit.Levels).insert(pair<int,Element*>(level+1,(Element*)curGate));
                    }
                }

            } 

            else cout << "something wrong" << endl;


        }
    }while (   (circuit.Levels).find(++level) != (circuit.Levels).end() ); 
    cout << "Levelization completed" << endl << endl;
    return true;
}





bool Resolve_Branches(Circuit& circuit)
{
   map<string,Wire*>:: iterator iter = (circuit.Netlist).begin();
   cout << "Resolve Branches started" << endl;
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

                cout << "Calling resolve wire for  " << iwire->id << endl;
                Resolve_Wire(circuit,iwire);
        }
        else 
        {
            cout << "No need to resolve wire " << iwire->id << endl;
        }
        iter++;
    }
    cout << "Resolve branches completed successfully" << endl << endl;
    return true;
}



void Resolve_Wire(Circuit&,Wire* wire)
{

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
        newname = Check_Name_Present(circuit,newname);
        // step 2. Add a new wire for this instance
        cout << "Adding derived wire:    " <<  newname  << endl;
        Add_Wire(circuit,newname.c_str(),CONNECTION);

        // The new wire's output should be the 
        // gate
        Add_Gate_To_Wire_Output(circuit,gate,newname.c_str());

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



string Check_Name_Present(Circuit& circuit,string givenname)
{
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




string intToString(int inInt)
{
    stringstream ss;
    string s;
    ss << inInt;
    s = ss.str();
    return s;
}


