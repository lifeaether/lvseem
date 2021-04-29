//
//  bp35a1.c
//  lvseem
//
//  Created by lifeaether on 2021/04/29.
//

#include "bp35a1.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

ssize_t bp35a1_write_bytes( const int fd, const uint8_t * const bytes, const size_t size )
{
    size_t write_size = 0;
    while ( write_size < size ) {
        const ssize_t n = write( fd, bytes + write_size, size - write_size );
        if ( n == -1 ) {
            fprintf( stderr, "write: %s\n", strerror( errno ) );
            break;
        }
        write_size += n;
    }
    return write_size;
}

bool bp35a1_write_string( const int fd, const char * const string )
{
    const size_t size = strlen( string );
    return bp35a1_write_bytes( fd, (const uint8_t *)string, size ) == size;
}

ssize_t bp35a1_read_bytes( const int fd, uint8_t * const bytes, const size_t size )
{
    size_t read_size = 0;
    while ( read_size < size ) {
        const ssize_t n = read( fd, bytes + read_size, size - read_size );
        if ( n == -1 ) {
            fprintf( stderr, "read: %s\n", strerror( errno ) );
            break;
        }
        read_size += n;
        if ( n == 0 ) {
            break;
        }
    }
    return read_size;
}

bool bp35a1_read_string( const int fd, char * const string, const size_t size )
{
    const ssize_t n = bp35a1_read_bytes( fd, (uint8_t *)string, size-1 );
    if ( n < 0 ) {
        return false;
    }
    string[n] = '\0';
    return true;
}

void bp35a1_read_to_end( const int fd )
{
    uint8_t buffer[64];
    while ( true ) {
        const ssize_t n = read( fd, buffer, sizeof( buffer ) );
        if ( n <= 0 ) {
            break;
        }
    }
}

void bp35a1_print_information( const int fd, FILE *fp )
{
    char response[128] = {};

    bp35a1_write_string( fd, "SKVER\r\n" );
    bp35a1_read_string( fd, response, sizeof(response) );
    fprintf( fp, "%s", response );

    bp35a1_write_string( fd, "SKAPPVER\r\n" );
    bp35a1_read_string( fd, response, sizeof(response) );
    fprintf( fp, "%s", response );

    bp35a1_write_string( fd, "SKINFO\r\n" );
    bp35a1_read_string( fd, response, sizeof(response) );
    fprintf( fp, "%s", response );

    const char *sksreg_names[] = {
        "S02", "S03", "S07", "S0A", "S15", "S16", "S17", "SA0", "SA1", "SFB", "SFD", "SFE", "SFF"
    };
    size_t sksreg_names_count = sizeof( sksreg_names ) / sizeof( sksreg_names[0] );

    for ( size_t i = 0; i < sksreg_names_count; ++i ) {
        char command[16] = {};
        sprintf( command, "SKSREG %s\r\n", sksreg_names[i] );
        bp35a1_write_string( fd, command );
        bp35a1_read_string( fd, response, sizeof(response) );
        fprintf( fp, "%s", response );
    }
}
