
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
#include "test.h"
using namespace std;



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



vector<Value> CaptureOutput()
{
    vector<Value> output;
    map<string,Wire *>::iterator iter = PriOutputs.begin();
    for (;iter != PriOutputs.end(); iter++)
        output.push_back((iter->second)->value);
    return output;
}


vector<Value> CaptureInput()
{
    vector<Value> output;
    map<string,Wire *>::iterator iter = PriInputs.begin();
    for (;iter != PriInputs.end(); iter++)
        output.push_back((iter->second)->value);
    return output;
}






