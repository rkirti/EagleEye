/*
 * DEBUGGIN MACROS
 */


#ifndef MACROS_H 
#define MACROS_H



static int atpgdebug=1;




#define ATPGPRINT(...) { if (atpgdebug) fprintf(__VA_ARGS__); fprintf(ATPG_DFILE,"\n"); }
#define PRINTDFRONTIER   {  list<WireValuePair>::iterator iter= circuit.DFrontier.begin();   \
                            ATPG_DFILE << "Printing DFrontier" << endl;      \
                            for (;iter!= circuit.DFrontier.end();iter++)  \
                            {      ATPG_DFILE << "Wire " <<  (((*iter).iwire)->id) <<  "  Value "  << (*iter).value << endl;  }  \
                            ATPG_DFILE << "DFrontier printed " << endl;      \
                         } 
#define PRINTJFRONTIER   {  list<WireValuePair>::iterator iter= circuit.JFrontier.begin();   \
                            ATPG_DFILE << "Printing JFrontier" << endl;      \
                            for (;iter!= circuit.JFrontier.end();iter++)  \
                            {      ATPG_DFILE << "Wire " <<  (((*iter).iwire)->id) <<  "  Value "  << (*iter).value << endl;  }  \
                            ATPG_DFILE << "JFrontier printed " << endl;      \
                         } 






#endif /* ifndef MACROS_H */
