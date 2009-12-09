/*
 * DEBUGGIN MACROS
 */


#ifndef MACROS_H 
#define MACROS_H



static int atpgdebug=1;
extern FILE* ATPG_DFILE;




#define ATPGPRINT(...) { if (atpgdebug) fprintf(__VA_ARGS__); fprintf(ATPG_DFILE,"\n"); }
#define PRINTDFRONTIER   {  list<WireValuePair>::iterator iter= circuit.DFrontier.begin();   \
                            fprintf(ATPG_DFILE,"Printing DFrontier\n");      \
                            for (;iter!= circuit.DFrontier.end();iter++)  \
                            {      fprintf(ATPG_DFILE,"Wire: %s   Value: %d\n", (((*iter).iwire)->id).c_str(), (*iter).value);    \
                                   fprintf(ATPG_DFILE,"DFrontier printed\n");      \
                         } }

#define PRINTJFRONTIER   { list<WireValuePair>::iterator iter= circuit.JFrontier.begin();   \
                            fprintf(ATPG_DFILE,"Printing JFrontier\n");      \
                            for (;iter!= circuit.JFrontier.end();iter++)  \
                            {      fprintf(ATPG_DFILE,"Wire: %s   Value: %d\n", (((*iter).iwire)->id).c_str(), (*iter).value);    \
                                   fprintf(ATPG_DFILE,"JFrontier printed\n");      \
                         } }









#endif /* ifndef MACROS_H */
