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

static uint8_t bp35a1_sksreg_sfe = 1;

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
    if ( size > 0 ) {
        string[n] = '\0';
    }
    return true;
}

bool pb35a1_read_line( const int fd, char * const string, const size_t size )
{
    bool result = true;
    size_t read_size = 0;
    while ( read_size < size-1 ) {
        const ssize_t n = read( fd, string + read_size, 1 );
        if ( n == -1 ) {
            fprintf( stderr, "read: %s\n", strerror( errno ) );
            result = false;
            break;
        }
        read_size += n;
        if ( n == 0 ) {
            break;
        }
        if ( read_size > 2 ) {
            if ( memcmp( string+read_size-2, "\r\n", 2 ) == 0 ) {
                break;
            }
        }
    }
    if ( size > 0 ) {
        string[read_size] = '\0';
    }
    return result;
}

bool bp35a1_read_to_end( const int fd, FILE *fp )
{
    char response[128] = {};
    while ( true ) {
        bp35a1_read_string( fd, response, sizeof( response ) );
        if ( strlen( response ) == 0 ) {
            break;
        }
        fprintf( stdout, "%s", response );
    }
    fprintf( stdout, "\n" );
    return true;
}

bool bp35a1_status( const int fd, FILE *fp )
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

    return true;
}

bool bp35a1_activescan( const int fd, FILE *fp )
{
    char command[128] = {};
    char response[128] = {};

    snprintf( command, sizeof( command ), "SKSCAN 2 FFFFFFFF 4\r\n" );
    bp35a1_write_string( fd, command );
    if ( bp35a1_sksreg_sfe ) {
        pb35a1_read_line( fd, response, sizeof( response ) );
        fprintf( fp, "%s", response );
    }
    for ( int i = 0; i < 10; ++i ) {
        pb35a1_read_line( fd, response, sizeof( response ) );
        fprintf( fp, "%s", response );
        const char event22[] = "EVENT 22";
        const char epandesc[] = "EPANDESC\r\n";
        if ( strncmp( response, event22, sizeof( event22 )-1 ) == 0 ) {
            return true;
        } else if ( strcmp( response, epandesc ) == 0 ) {
            pb35a1_read_line( fd, response, sizeof( response ) );
            fprintf( fp, "%s", response );  // Channel
            pb35a1_read_line( fd, response, sizeof( response ) );
            fprintf( fp, "%s", response );  // Channel Page
            pb35a1_read_line( fd, response, sizeof( response ) );
            fprintf( fp, "%s", response );  // Pan ID
            pb35a1_read_line( fd, response, sizeof( response ) );
            fprintf( fp, "%s", response );  // Addr
            pb35a1_read_line( fd, response, sizeof( response ) );
            fprintf( fp, "%s", response );  // LQI
            pb35a1_read_line( fd, response, sizeof( response ) );
            fprintf( fp, "%s", response );  // PairID
        }
        sleep( 1 );
    }
    return false;
}

bool bp35a1_set_b_id( const int fd, FILE *fp, const char *b_id )
{
    char command[64] = {};
    char response[64] = {};
    snprintf( command, sizeof( command ), "SKSETRBID %s\r\n", b_id );
    bp35a1_write_string( fd, command );
    if ( bp35a1_sksreg_sfe ) {
        pb35a1_read_line( fd, response, sizeof( response ) );
        fprintf( stdout, "%s", response );
    }
    pb35a1_read_line( fd, response, sizeof( response ) );
    fprintf( stdout, "%s", response );
    if ( strcmp( response, "OK\r\n" ) != 0 ) {
        return false;
    }
    return true;
}

bool bp35a1_set_b_password( const int fd, FILE *fp, const char *b_password )
{
    char command[64] = {};
    char response[64] = {};
    snprintf( command, sizeof( command ), "SKSETPWD C %s\r\n", b_password );
    bp35a1_write_string( fd, command );
    if ( bp35a1_sksreg_sfe ) {
        pb35a1_read_line( fd, response, sizeof( response ) );
        fprintf( stdout, "%s", response );
    }
    pb35a1_read_line( fd, response, sizeof( response ) );
    fprintf( stdout, "%s", response );
    if ( strcmp( response, "OK\r\n" ) != 0 ) {
        return false;
    }
    return true;
}

bool bp35a1_pana_connect( const int fd, FILE *fp, const char *channel, const char *pan_id, const char *addr )
{
    return false;
}
