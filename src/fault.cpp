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
#include "fault.h"
using namespace std;




/**
 * Reads a file specifying the faults into the fault set data structure.
 * The file should have the faults in the following format -
 * one fault on each line, specifying faulty wire name and stuck at value
 * (stuck-at-0 or stuck-at-1) separated by a space.
 * The data structure passed on to this function should ideally be the 
 * set field of the test object which is being run currently.
 *
 * @param fname
 * Name of the file to read the faults from
 *
 * @param circuit
 * Reference to the circuit object for which the faults are specified.
 * Checks are implemented here to see if the fault set is valid. If a wire not belonging to this circuit is specified in the faults file
 * an error is thrown and the program exits.
 *
 * @param faultset
 * A list of Fault objects to read the faults into.
 * @see Check fault.h for definition of the fault class.
 */

bool Read_Faults_Into_FaultSet(string fname, Circuit& circuit,FaultSet& set)
{
    ifstream faultsFile;
    
    faultsFile.open(fname.c_str(),ios::in);
    if (!faultsFile.good())
    {
        cout << "Couldn't open the file " << fname << endl;
        exit(-1);
    }
    
    string wireName;
    int faultType;
    map<string,Wire *>::iterator iter;

    while (faultsFile >> wireName >> faultType)
    {
        
        cout << "Fault specified: Wire: " << wireName << " FaultValue " << faultType << endl;
        if ( ((iter = circuit.Netlist.find(wireName)) !=  circuit.Netlist.end()) &&  (faultType == 0 || faultType == 1))
        {
            Fault readFault(iter->second, (faultType==0)?0:1);
            set.push_back(readFault);
        }
        else
        {
            cout << "Fault specified in " << fname << " incorrect - wire not found or faultvalue incorrect" << endl; 
            exit(-1);
        }
    }

    faultsFile.close();
    return true;
}


/**
 * @param 
 * List of faults which is to be printed. Prints
 * out the name of the faulty wire, the stuck at value 
 * and whether the fault is detected or not at the time of 
 * invocation of this function.
 */

void Print_FaultSet(FaultSet set)
{
    list<Fault>::iterator iter;
    cout << "Printing the set of faults " << endl; 
    cout << "Wire name \t" << "Fault Type \t" << "Detected" << endl;
    for (iter = set.begin(); iter != set.end(); iter++)
    { 
        cout << ((*iter).FaultSite)->id << "\t" << (*iter).faultType << "\t" << (*iter).detected << endl; 
    }
    cout << endl;
    return;
}

