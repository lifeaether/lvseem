//
//  bp35a1.c
//  lvseem
//
//  Created by lifeaether on 2021/04/29.
//

#include "bp35a1.h"

#include <unistd.h>
#include <string.h>

static bool parse_read_to( const int fd, void * const read_bytes, size_t *read_size, const void * const to_bytes, const size_t to_size )
{
    for ( size_t i = 0; i < *read_size; i++ ) {
        if ( read( fd, read_bytes+i, 1 ) != 1 ) {
            *read_size = i+1;
            return false;
        }
        if ( i >= to_size-1 && memcmp( read_bytes+i-to_size+1, to_bytes, to_size ) == 0 ) {
            *read_size = i+1;
            return true;
        }
    }
    return false;
}

const char bp35a1_end_of_line[] = "\r\n";
const size_t bp35a1_end_of_line_size = sizeof( bp35a1_end_of_line );

const char bp35a1_command_skver[] = "SKVER";
const size_t bp35a1_command_skver_size = sizeof( bp35a1_command_skver );
static bool parse_skver( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    char bytes[64];
    size_t byte_size = sizeof( bytes );
    if ( ! parse_read_to( fd, bytes, &byte_size, bp35a1_end_of_line, bp35a1_end_of_line_size-1 ) ) {
        return false;
    }
    return true;
}

const char bp35a1_event_ever[] = "EVER";
const size_t bp35a1_event_ever_size = sizeof( bp35a1_event_ever );
static bool parse_ever( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    {
        char bytes[1];
        size_t byte_size = sizeof( bytes );
        if ( ! parse_read_to( fd, bytes, &byte_size, " ", 1 ) ) {
            return false;
        }
    }
    {
        char bytes[64];
        size_t byte_size = sizeof( bytes );
        if ( ! parse_read_to( fd, bytes, &byte_size, bp35a1_end_of_line, bp35a1_end_of_line_size-1 ) ) {
            return false;
        }
        if ( handler->event_ever ) {
            bytes[byte_size-bp35a1_end_of_line_size+1] = '\0';
            if ( ! handler->event_ever( userdata, bytes ) ) {
                return false;
            }
        }
    }
    return true;
}

bool bp35a1_write( const int fd, const void * const bytes, const size_t size )
{
    for ( size_t current_size = 0; current_size < size; ) {
        const ssize_t n = write( fd, bytes, size - current_size );
        if ( n == -1 ) {
            return false;
        }
        current_size += n;
    }
    return true;
}

bool bp35a1_parse( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    char buffer[64] = {};
    const size_t buffer_size = sizeof( buffer );
    for ( size_t i = 0; i < buffer_size; i++ ) {
        const ssize_t n = read( fd, buffer+i, 1 );
        if ( n == -1 ) {
            return false;
        } else if ( n == 0 ) {
            return false;
        }
        if ( strcmp( buffer, bp35a1_command_skver ) == 0 ) {
            if ( parse_skver( fd, handler, userdata ) ) {
                return true;
            }
        } else if ( strcmp( buffer, bp35a1_event_ever ) == 0 ) {
            if ( parse_ever( fd, handler, userdata ) ) {
                return true;
            }
        }
    }
    return false;
}
