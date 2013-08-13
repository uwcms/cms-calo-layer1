#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int main ( int argc, char** argv ) {
    int fin;
    int fout;

    char buf [100];

    if ( argc != 3 ) {
        printf("Usage: vme2fd [instream] [outstream]\n");
        exit(0);
    }

    fin  = open( argv[1], O_RDONLY );
    fout = open( argv[2], O_WRONLY );

    read( fin, buf, 100 );
    write( fout, buf, strlen(buf) );

    close( fin );
    close( fout );

    return 0;
}
