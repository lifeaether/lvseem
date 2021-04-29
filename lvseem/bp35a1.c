//
//  bp35a1.c
//  lvseem
//
//  Created by lifeaether on 2021/04/29.
//

#include "bp35a1.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static int bp35a1_sksreg_sfe = 1; // echo back.

static const uint8_t bp35a1_end_of_line[] = { '\r', '\n' };

bool bp35a1_command_write( const int fd, const char *input )
{
    const size_t input_length = strlen( input );
    size_t write_size = 0;
    while ( write_size < input_length ) {
        const ssize_t n = write( fd, input+write_size, input_length-write_size );
        if ( n == -1 ) {
            fprintf( stderr, "write: %s\n", strerror( errno ) );
            return false;
        }
        write_size += n;
    }
    return true;
}

bool bp35a1_command_write_command( const int fd, const char *command )
{
    if ( ! bp35a1_command_write( fd, command ) ) {
        return false;
    }
    if ( ! bp35a1_command_write( fd, (const char *)bp35a1_end_of_line ) ) {
        return false;
    }
    return true;
}

bool bp35a1_command_read_line( const int fd, uint8_t * const response, size_t *size )
{
    const size_t response_size = *size;
    *size = 0;
    while ( *size < response_size ) {
        const ssize_t n = read( fd, response + *size, 1 );
        if ( n == -1 ) {
            fprintf( stderr, "read: %s\n", strerror( errno ) );
            return false;
        } else if ( n == 0 ) {
            return false;
        }
        *size += n;
        if ( memcmp( response + *size - sizeof(bp35a1_end_of_line), bp35a1_end_of_line, sizeof(bp35a1_end_of_line) ) == 0 ) {
            return true;
        }
    }
    return false;
}

bool bp35a1_command_read_ok( const int fd )
{
    const uint8_t ok[] = { 'O', 'K', '\r', '\n' };
    uint8_t resposne[sizeof(ok)] = {};
    size_t resposne_size = sizeof( resposne_size );
    if ( ! bp35a1_command_read_line( fd, resposne, &resposne_size ) ) {
        return false;
    }
    return memcmp( resposne, ok, sizeof(ok) ) == 0;
}

bool bp35a1_command( const int fd, const char *command, char *response, size_t size )
{
    if ( ! bp35a1_command_write_command( fd, command ) ) {
        return false;
    }

    if ( bp35a1_sksreg_sfe ) {
        const size_t command_size = strlen( command );
        size_t response_size = command_size + sizeof( bp35a1_end_of_line );
        uint8_t *response = malloc( response_size );
        if ( ! bp35a1_command_read_line( fd, response, &response_size ) ) {
            return false;
        }
        if ( memcmp( response, command, command_size ) != 0 ) {
            return false;
        }
    }

    {
        if ( ! bp35a1_command_read_line( fd, (uint8_t *)response, &size ) ) {
            return false;
        }
        response[size - sizeof( bp35a1_end_of_line )] = '\0';
    }

    return bp35a1_command_read_ok( fd );
}
