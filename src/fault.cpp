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




bool Read_Faults_Into_FaultSet(string fname, Circuit& circuit,FaultSet& set)
{
    ifstream faultsFile;
    
    faultsFile.open(fname.c_str(),ios::in);
    // Read faults from the faults.txt file
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

