#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "endetool.h"

using namespace std;

const char ekey[] = "12345678901234567890123456789012";
const char eiv[]  = "09876543210987654321098765432109";
const char teststr[] = "Testing_source_words_for_libendetool_and_endecmd.";

void test1()
{
    printf( "> creating ende tool : " ); fflush( stdout );
    EnDeTool* ende = new EnDeTool();

    if( ende != NULL )
    {
        printf( "Ok.\n" );
        printf( "> Configure : " );
        fflush( stdout );
        ende->cryptkey( ekey, eiv );

        printf( "Ok.\n" );
        fflush( stdout );

        printf( "> source string : %s\n", teststr );
        fflush( stdout );

        ende->text( teststr );
        printf( "> encoding ... " );
        fflush( stdout );
        string encstr;
        const char* rets = ende->encodedtext();
        if ( rets != NULL )
            encstr = rets;
        printf( "> encoded : %s\n", encstr.c_str() );
        fflush( stdout );

        ende->encodedtext( encstr.c_str() );
        string decstr = ende->text();
        printf( "> decoded : %s\n", decstr.c_str() );
        fflush( stdout );

        delete ende;
    }
    else
    {
        printf( "failure.\n" );
        fflush( stdout );
    }
}

int main( int argc, char** argv )
{
    printf( "libendetool testing.\n" );
    fflush( stdout );

    printf( "TESTING 1: plain texts.\n");
    fflush( stdout );
    test1();

    fflush( stdout );
    system( "pause" );

    return 0;
}
