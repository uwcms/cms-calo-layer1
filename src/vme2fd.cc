#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "caen.h"
#include "VMEStream.h"

int main ( int argc, char** argv )
{
    int fin;
    int fout;

    char buf [100];

    if ( argc != 3 ) {
        printf("Usage: vme2fd [instream] [outstream]\n");
        exit(0);
    }

    fin  = open( argv[1], O_RDONLY );
    fout = open( argv[2], O_WRONLY );

    ssize_t bytes_read = read( fin, buf, 100 );
    write( fout, buf, bytes_read );

    close( fin );
    close( fout );

    return 0;
}
