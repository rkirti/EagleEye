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
#include "helper.h"
using namespace std;


extern ofstream CIRCUIT_DFILE;
extern ofstream EVALUATE_DFILE;
extern ofstream ATPG_DFILE;
extern ofstream MAIN_DFILE;




// returns if the value is known or not
// 
bool isNotKnown(Value v)
{
    switch(v)
     {
	    case ONE:
	    case ZERO:
	    case D:
	    case DBAR:
	    return false;
	
	default:
	    return true;
     }
     return true;
}


int Translate_Value_To_Int(Value value)
{
    switch(value)
    {
        case 0x0: 
            return 0;
        case 0xff:
            return 1;
    }
    return 0;
}



/*Use only to check if value is c xor i or  cbar xor i*/
Value Do_Xor(Value val1, Value val2)
{

    Value output;
    Value negval1 =  (Value) (val1 != U)?(Value)(~(val1)&0xf):U;
    Value negval2 =  (Value) (val2 != U)?(Value)(~(val2)&0xf):U;
    output = (Value)((( (val1 & negval2) | (negval1 & val2)))&0xf);
    return output;
}


string intToString(int inInt)
{
    stringstream ss;
    string s;
    ss << inInt;
    s = ss.str();
    return s;
}


void Init_Debug()
{
    //CIRCUIT_DFILE.open("/dev/null",ios::out);   
    //EVALUATE_DFILE.open("/dev/null",ios::out);   
    //ATPG_DFILE.open("/dev/null",ios::out);   
    //MAIN_DFILE.open("/dev/null",ios::out);   

    CIRCUIT_DFILE.open("debug/ckt.debug",ios::out);   
    EVALUATE_DFILE.open("debug/eval.debug",ios::out);   
    ATPG_DFILE.open("debug/atpg.debug",ios::out);   
    MAIN_DFILE.open("debug/main.debug",ios::out);   
}



void Print_All_Wires(Circuit& circuit)
{
    CIRCUIT_DFILE << "Print_All_Wires called"  << endl;


    // Print out the primary inputs
    CIRCUIT_DFILE << "Printing all the PIs "   << endl << endl;
    map<string,Wire*> ::iterator iter =  (circuit.PriInputs).begin();

    for (;iter != (circuit.PriInputs).end(); iter++)
    {   
        CIRCUIT_DFILE << setw(20) << (iter->second)->id << ":    " << (iter->second)->value << endl;
    }


    // Print out the primary outputs
    iter =  (circuit.PriOutputs).begin();
    CIRCUIT_DFILE << "Printing all POs " << endl << endl;
    for (;iter != (circuit.PriOutputs).end(); iter++)
    {   
        CIRCUIT_DFILE  << setw(20) << (iter->second)->id << ":    " << (iter->second)->value << endl;
    }


    // Print out the netlist
    iter =  (circuit.Netlist).begin();
    CIRCUIT_DFILE << "Printing out the netlist " << endl << endl;
    for (;iter != (circuit.Netlist).end(); iter++)
    {   
        CIRCUIT_DFILE << setw(20) << (iter->second)->id << ":    " << (iter->second)->value << endl;
    }

    CIRCUIT_DFILE <<  endl << endl;
}



/*
 * Clear all wire values to U
 */
void Clear_Wire_Values(Circuit& circuit)
{

    // Print out the netlist
    // Clear the values of all the wires. Set the value to U
    map<string,Wire*> ::iterator iter =  (circuit.Netlist).begin();
    for (;iter != (circuit.Netlist).end(); iter++)
    {   
        (iter->second)->value = U;
    }
}



/*
 * Clear all internal wire values(all wires except PIs) to U
 */
void Clear_Internal_Wire_Values( Circuit& circuit)
{

    // Print out the netlist
    // Clear the values of all the wires. Set the value to U
    map<string,Wire*> ::iterator iter =  (circuit.Netlist).begin();
    for (;iter != (circuit.Netlist).end(); iter++)
    {   
        if ( (iter->second)->wtype != PI)
            (iter->second)->value = U;
    }
}


