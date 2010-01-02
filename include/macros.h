/*
 * DEBUGGIN MACROS
 */


#ifndef MACROS_H 
#define MACROS_H



static int atpgdebug=1;




#define ATPGPRINT(...) { if (atpgdebug) fprintf(__VA_ARGS__); fprintf(ATPG_DFILE,"\n"); }
#define PRINTDFRONTIER   {  list<WireValuePair>::iterator iter= circuit.DFrontier.begin();   \
                            ATPG_DFILE << "Printing Global DFrontier" << endl;      \
                            for (;iter!= circuit.DFrontier.end();iter++)  \
                            {      ATPG_DFILE << "Wire " <<  (((*iter).iwire)->id) <<  "  Value "  << (*iter).value << endl;  }  \
                            ATPG_DFILE << "Global DFrontier printed " << endl;      \
                             ATPG_DFILE << "Printing Temp DFrontier" << endl;      \
   for ( iter = circuit.TempDFrontier.begin();iter!= circuit.TempDFrontier.end();iter++)  \
                            {      ATPG_DFILE << "Wire " <<  (((*iter).iwire)->id) <<  "  Value "  << (*iter).value << endl;  }  \
                            ATPG_DFILE << "Temp DFrontier printed " << endl;      \
                         } 





#define PRINTJFRONTIER   {  list<WireValuePair>::iterator iter= circuit.JFrontier.begin();   \
                            ATPG_DFILE << "Printing Global JFrontier" << endl;      \
                            for (;iter!= circuit.JFrontier.end();iter++)  \
                            {      ATPG_DFILE << "Wire " <<  (((*iter).iwire)->id) <<  "  Value "  << (*iter).value << endl;  }  \
                            ATPG_DFILE << "Global JFrontier printed " << endl;      \
                             ATPG_DFILE << "Printing Temp JFrontier" << endl;      \
   for ( iter = circuit.TempJFrontier.begin();iter!= circuit.TempJFrontier.end();iter++)  \
                            {      ATPG_DFILE << "Wire " <<  (((*iter).iwire)->id) <<  "  Value "  << (*iter).value << endl;  }  \
                            ATPG_DFILE << "Temp JFrontier printed " << endl;      \
                         }
                         






#endif /* ifndef MACROS_H */
