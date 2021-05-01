#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "endetool.h"

///////////////////////////////////////////////////////////////////////////

using namespace std;

///////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
	#define TARGET_PF	"WIN32"
#else
	#define TARGET_PF	"POSIX"
#endif

///////////////////////////////////////////////////////////////////////////

string	me_path;
string	me_name;
string	me_version = "0.2.5.13";
string	ende_key;
string	ende_data;
string  ende_file_src;
string  ende_file_dst;
string  ende_srcfile;

bool    opt_decode = false;
bool	opt_verbose_off = false;
bool	opt_nobase64 = false;
bool    opt_showhelp = false;
bool	opt_file = false;
bool    opt_compress = false;
string  opt_outfile;

///////////////////////////////////////////////////////////////////////////

const std::string WHITESPACE = " \n\r\t";

string TrimLeft(const std::string& s)
{
    size_t startpos = s.find_first_not_of( WHITESPACE );
    return (startpos == std::string::npos) ? "" : s.substr(startpos);
}

string TrimRight(const std::string& s)
{
    size_t endpos = s.find_last_not_of( WHITESPACE );
    return (endpos == std::string::npos) ? "" : s.substr(0, endpos+1);
}

string Trim(const std::string& s)
{
    return TrimRight(TrimLeft(s));
}

void readfile()
{
    ende_srcfile = ende_data;
    ende_data.clear();

    FILE* fp = fopen( ende_srcfile.c_str(), "rb" );
    if ( fp != NULL )
    {
        fseek( fp, 0L, SEEK_END );
        unsigned fsz = ftell( fp );
        rewind( fp );

        if ( fsz > 0 )
        {
            char* fbuff = new char[ fsz ];
            if ( fbuff != NULL )
            {
                fread( fbuff, 1, fsz, fp );
                ende_data.assign( fbuff, fsz );
            }
            fclose( fp );
        }
    }
}

bool writefile( const char* b, int l )
{
    if ( ( b == NULL ) || ( l <= 0 ) ) 
        return false;

    if ( opt_outfile.size() == 0 )
    {
        opt_outfile = ende_srcfile + ".out";
    }

    FILE* fp = fopen( opt_outfile.c_str(), "wb" );
    if ( fp != NULL )
    {
        fwrite( b, 1, l , fp );
        fclose( fp );

        return true;
    }

    return false;
}

void parseArgs( int argc, char** argv )
{
	for( int cnt=0; cnt<argc; cnt++ )
	{
		string strtmp = argv[ cnt ];
		size_t fpos   = string::npos;

		if ( cnt == 0 )
		{
			fpos = strtmp.find_last_of( "\\" );

			if ( fpos == string::npos )
			{
				fpos = strtmp.find_last_of( "/" );
			}

			if ( fpos != string::npos )
			{
				me_path = strtmp.substr( 0, fpos );
				me_name = strtmp.substr( fpos + 1 );
			}
			else
			{
				me_name = strtmp;
			}
		}
		else
		{
			if ( strtmp.find( "--key=" ) == 0 )
			{
				ende_key = strtmp.substr( 6 );
			}
			else
			if ( strtmp.find( "--verboseoff" ) == 0 )
			{
				opt_verbose_off = true;
			}
			else
			if ( strtmp.find ( "--nobase64" ) == 0 )
			{
				opt_nobase64 = true;
			}
			else
			if ( strtmp.find( "--help" ) == 0 )
			{
				opt_showhelp = true;
			}
			else
			if ( strtmp.find( "--decode" ) == 0 )
			{
				opt_decode = true;
			}
			else
			if ( strtmp.find( "--compress" ) == 0 )
			{
				opt_compress = true;
			}
            else
            if ( strtmp.find( "--file" ) == 0 )
            {
                opt_file = true;
            }
			else
			{
                if ( ende_data.size() == 0 )
                {
                    ende_data = strtmp;
                }
                else
                {
                    opt_outfile = strtmp;
                }
			}
		}
	}
}

void printAbout()
{
	if ( opt_verbose_off == false )
	{
		printf( "%s : In-line command shell tool %s, verison %s\n",
				me_name.c_str(),
				TARGET_PF,
				me_version.c_str() );
		printf( "(C)Copyrighted 2013 - 2021 Raphael Kim\n" );
		printf( "\n" );
	}
}

void printArgsHelp()
{
	if ( opt_verbose_off == false )
	{
		printf( "\tusage : %s [ options ] (string or source file) (out file)\n", me_name.c_str() );
		printf( "\n" );
		printf( "\t_options_\n" );
		printf( "\n" );
		printf( "\t  --key=(encoding key for maximum 32 charactors)\n" );
		printf( "\t  --verboseoff : turns off all information except en/decoded.\n" );
		printf( "\t  --decode     : decodes source string\n" );
		printf( "\t  --compress   : compressed encryption\n" );
        printf( "\t  --file       : open file\n" );
		printf( "\n" );
	}
}

int main( int argc, char** argv )
{
	parseArgs( argc, argv );
	printAbout();

	if ( ( ende_data.size() == 0 ) || ( opt_showhelp == true )
	     || ( ende_key.size() == 0 ) )
	{
		printArgsHelp();
		return 0;
	}

    if ( opt_file == true )
    {
        readfile();
        if ( ende_data.size() == 0 )
        {
            if ( opt_verbose_off == false )
            {
                printf( "Failed to read file : %s\n", 
                        ende_srcfile.c_str() );
            }
            return -1;
        }
    }

#ifdef DEBUG
    printf( "debug-key:[%s]\n", ende_key.c_str() );
#endif /// of DEBUG
	EnDeTool* ende = new EnDeTool();
	if ( ende != NULL )
	{
		ende->reset();
		ende->compress( opt_compress );
		ende->cryptkey( ende_key.c_str() );
		
		if( opt_decode == true )
		{
            if ( opt_file == false )
            {
                ende->encodedtext( ende_data.c_str() );
                
                const char* strdec = ende->text();

                if ( strdec != NULL )
                {
                    string strtrimmed = Trim( strdec );
                    printf( "%s", strtrimmed.c_str() );
                }
                else
                {
                    if ( opt_verbose_off == false )
                    {
                        printf( "decode failure.\n" );
                    }
                }
            }
            else
            {
                char* outb = NULL;
                int esz = ende->decodebinary( ende_data.c_str(), 
                                              ende_data.size(),
                                              outb );
                if ( ( esz > 0 ) && ( outb != NULL ) )
                {
                    writefile( outb, esz );
                    delete[] outb;
                }
            }
		}
		else
		{
            if ( opt_file == false )
            {
                ende->text( ende_data.c_str() );

                const char* strenc = ende->encodedtext();

                if ( strenc != NULL )
                {
                    string strtrimmed = Trim( strenc );
                    printf( "%s", strtrimmed.c_str() );
                }
            }
            else
            {
                char* outb = NULL;
                int esz = ende->encodebinary( ende_data.c_str(),
                                              ende_data.size(),
                                              outb );
                if ( ( esz > 0 ) && ( outb != NULL ) )
                {
                    writefile( outb, esz );
                    delete[] outb;
                }
            }
		}

		if ( opt_verbose_off == false )
		{
			printf( "\n" );
		}

		ende->reset();

		delete ende;
	}

	return 0;
}
