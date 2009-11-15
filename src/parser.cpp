/*
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
#include "parser.h"
using namespace std;


static Value And(list<Wire*> inputs)
{
   Value output=U;
   Wire::iterator iter;
   for (iter=inputs.begin(); iter != inputs.end(), iter++ )
       output &= (*iter).value;
    return output;

}


static Value Or(list<Wire*> inputs)
{
   Value output=U;
   Wire::iterator iter;
   for (iter=inputs.begin(); iter != inputs.end(), iter++ )
       output |= (*iter).value;
    return output;

}


static Value Nand(list<Wire*> inputs)
{
   Value output=U;
   Wire::iterator iter;
   for (iter=inputs.begin(); iter != inputs.end(), iter++ )
       output &= (*iter).value;
    return ((~output)&0xf);

}


static Value Nor(list<Wire*> inputs)
{
   Value output=U;
   Wire::iterator iter;
   for (iter=inputs.begin(); iter != inputs.end(), iter++ )
       output |= (*iter).value;
    return ((~output)&0xf);

}




Value Gate::Evaluate()
{
    return g_EvaluateTable[gtype](inputs);
}


