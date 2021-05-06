//
//  bp35a1.c
//  lvseem
//
//  Created by lifeaether on 2021/04/29.
//

#include "bp35a1.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static bool bp35a1_write( const int fd, const void * const bytes, const size_t size )
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

static bool read_bytes_to( const int fd, void * const read_bytes, size_t *read_size, const void * const to_bytes, const size_t to_size )
{
    for ( size_t i = 0; i < *read_size; i++ ) {
        if ( read( fd, read_bytes+i, 1 ) != 1 ) {
            *read_size = i+1;
            return false;
        }
        if ( i >= to_size-1 && memcmp( read_bytes+i-to_size+1, to_bytes, to_size ) == 0 ) {
            *read_size = i-to_size+1;
            return true;
        }
    }
    return false;
}

static bool read_bytes( const int fd, const void * const bytes, size_t size )
{
    for ( int i = 0; i < size; i++ ) {
        uint8_t b = 0;
        if ( read( fd, &b, 1 ) != 1 ) {
            return false;
        }
        if ( b != ((const uint8_t * const)bytes)[i] ) {
            return false;
        }
    }
    return true;
}

static bool read_string_to( const int fd, char * const read_string, const size_t read_size, const void * const to_string )
{
    size_t size = read_size-1;
    if ( ! read_bytes_to( fd, read_string, &size, to_string, strlen( to_string ) ) ) {
        return false;
    }
    if ( size > 0 ) {
        read_string[size] = '\0';
    }
    return true;
}

static bool read_string( const int fd, const char * const string )
{
    return read_bytes( fd, string, strlen( string ) );
}

static bool parse_whitespace( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    return true;
}

static bool parse_skver( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    char string[64] = {};
    if ( ! read_string_to( fd, string, sizeof( string ), "\r\n" ) ) {
        return false;
    }
    return true;
}

static bool parse_ever( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    if ( ! read_string( fd, " " ) ) {
        return false;
    }
    {
        char string[64] = {};
        if ( ! read_string_to( fd, string, sizeof( string ), "\r\n" ) ) {
            return false;
        }
        if ( ! handler->event_ever ) {
            return false;
        }
        if ( ! handler->event_ever( userdata, string ) ) {
            return false;
        }
    }
    if ( ! read_string( fd, "OK\r\n" ) ) {
        return false;
    }
    return true;
}

static bool parse_skinfo( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    char string[64] = {};
    if ( ! read_string_to( fd, string, sizeof( string ), "\r\n" ) ) {
        return false;
    }
    return true;
}

static bool parse_einfo( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    if ( ! read_string( fd, " " ) ) {
        return false;
    }

    char ipaddr[64] = {};
    if ( ! read_string_to( fd, ipaddr, sizeof( ipaddr ), " " ) ) {
        return false;
    }

    char addr64[32] = {};
    if ( ! read_string_to( fd, addr64, sizeof( addr64 ), " " ) ) {
        return false;
    }

    char channel[8] = {};
    if ( ! read_string_to( fd, channel, sizeof( channel ), " " ) ) {
        return false;
    }

    char panid[8] = {};
    if ( ! read_string_to( fd, panid, sizeof( panid ), " " ) ) {
        return false;
    }

    char addr16[8] = {};
    if ( ! read_string_to( fd, addr16, sizeof( addr16 ), "\r\n" ) ) {
        return false;
    }

    if ( ! handler->event_einfo ) {
        return false;
    }

    if ( ! handler->event_einfo( userdata, ipaddr, addr64, channel, panid, addr16 ) ) {
        return false;
    }

    if ( ! read_string( fd, "OK\r\n" ) ) {
        return false;
    }

    return true;
}

static bool parse_skscan( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    char string[64] = {};
    if ( ! read_string_to( fd, string, sizeof( string ), "\r\n" ) ) {
        return false;
    }
    if ( ! read_string( fd, "OK\r\n" ) ) {
        return false;
    }
    return true;
}

static bool parse_sksetpwd( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    char string[128] = {};
    if ( ! read_string_to( fd, string, sizeof( string ), "\r\n" ) ) {
        return false;
    }
    if ( ! read_string( fd, "OK\r\n" ) ) {
        return false;
    }
    return true;
}

static bool parse_sksetrbid( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    char string[128] = {};
    if ( ! read_string_to( fd, string, sizeof( string ), "\r\n" ) ) {
        return false;
    }
    if ( ! read_string( fd, "OK\r\n" ) ) {
        return false;
    }
    return true;
}

static bool parse_sksreg( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    char string[128] = {};
    if ( ! read_string_to( fd, string, sizeof( string ), "\r\n" ) ) {
        return false;
    }
    if ( ! read_string( fd, "OK\r\n" ) ) {
        return false;
    }
    return true;
}

static bool parse_skll64( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    char string[128] = {};
    if ( ! read_string_to( fd, string, sizeof( string ), "\r\n" ) ) {
        return false;
    }

    if ( ! read_string_to( fd, string, sizeof( string ), "\r\n" ) ) {
        return false;
    }

    if ( ! handler->response_ll64 ) {
        return false;
    }

    if ( ! handler->response_ll64( userdata, string ) ) {
        return false;
    }

    return true;
}

static bool parse_skjoin( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    char string[128] = {};
    if ( ! read_string_to( fd, string, sizeof( string ), "\r\n" ) ) {
        return false;
    }
    if ( ! read_string( fd, "OK\r\n" ) ) {
        return false;
    }
    return true;
}

static bool parse_event_1f( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    if ( ! read_string( fd, " " ) ) {
        return false;
    }

    char sender[64] = {};
    if ( ! read_string_to( fd, sender, sizeof( sender ), "\r\n" ) ) {
        return false;
    }

    if ( ! handler->event_1f ) {
        return false;
    }

    if ( ! handler->event_1f( userdata, sender ) ) {
        return false;
    }

    return true;
}

static bool parse_event_20( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    if ( ! read_string( fd, " " ) ) {
        return false;
    }

    char string[128] = {};
    if ( ! read_string_to( fd, string, sizeof( string ), "\r\n" ) ) {
        return false;
    }

    if ( ! handler->event_20 ) {
        return false;
    }

    if ( ! handler->event_20( userdata, string ) ) {
        return false;
    }

    return true;
}

static bool parse_event_21( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    if ( ! read_string( fd, " " ) ) {
        return false;
    }

    char sender[128] = {};
    if ( ! read_string_to( fd, sender, sizeof( sender ), " " ) ) {
        return false;
    }

    char side[8] = {};
    if ( ! read_string_to( fd, side, sizeof( side ), "\r\n" ) ) {
        return false;
    }

    if ( ! handler->event_21 ) {
        return false;
    }

    if ( ! handler->event_21( userdata, sender, side ) ) {
        return false;
    }

    return true;
}

static bool parse_event_22( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    if ( ! read_string( fd, " " ) ) {
        return false;
    }

    char sender[128] = {};
    if ( ! read_string_to( fd, sender, sizeof( sender ), "\r\n" ) ) {
        return false;
    }

    if ( ! handler->event_22 ) {
        return false;
    }

    if ( ! handler->event_22( userdata, sender ) ) {
        return false;
    }

    return true;
}

static bool parse_event_25( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    if ( ! read_string( fd, " " ) ) {
        return false;
    }

    char sender[128] = {};
    if ( ! read_string_to( fd, sender, sizeof( sender ), "\r\n" ) ) {
        return false;
    }

    if ( ! handler->event_25 ) {
        return false;
    }

    if ( ! handler->event_25( userdata, sender ) ) {
        return false;
    }

    return true;
}

static bool parse_eedscan( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    if ( ! read_string( fd, "\r\n" ) ) {
        return false;
    }
    
    char string[256] = {};
    if ( ! read_string_to( fd, string, sizeof( string ), "\r\n" ) ) {
        return false;
    }

    if ( ! handler->event_eedscan ) {
        return false;
    }

    if ( ! handler->event_eedscan( userdata, string ) ) {
        return false;
    }

    if ( ! read_string( fd, "\r\n" ) ) {
        return false;
    }

    return true;
}

static bool parse_epandesc( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    if ( ! read_string( fd, "\r\n" ) ) {
        return false;
    }

    char buffer[256] = {};
    if ( ! read_string_to( fd, buffer, sizeof( buffer ), "Channel:" ) ) {
        return false;
    }

    char channel[8] = {};
    if ( ! read_string_to( fd, channel, sizeof( channel ), "\r\n" ) ) {
        return false;
    }

    if ( ! read_string_to( fd, buffer, sizeof( buffer ), "Channel Page:" ) ) {
        return false;
    }

    char channel_page[8] = {};
    if ( ! read_string_to( fd, channel_page, sizeof( channel_page ), "\r\n" ) ) {
        return false;
    }

    if ( ! read_string_to( fd, buffer, sizeof( buffer ), "Pan ID:" ) ) {
        return false;
    }

    char panid[8] = {};
    if ( ! read_string_to( fd, panid, sizeof( panid ), "\r\n" ) ) {
        return false;
    }

    if ( ! read_string_to( fd, buffer, sizeof( buffer ), "Addr:" ) ) {
        return false;
    }

    char addr[32] = {};
    if ( ! read_string_to( fd, addr, sizeof( addr ), "\r\n" ) ) {
        return false;
    }

    if ( ! read_string_to( fd, buffer, sizeof( buffer ), "LQI:" ) ) {
        return false;
    }

    char lqi[8] = {};
    if ( ! read_string_to( fd, lqi, sizeof( lqi ), "\r\n" ) ) {
        return false;
    }

    if ( ! read_string_to( fd, buffer, sizeof( buffer ), "PairID:" ) ) {
        return false;
    }

    char pairid[16] = {};
    if ( ! read_string_to( fd, pairid, sizeof( pairid ), "\r\n" ) ) {
        return false;
    }

    if ( handler->event_epandesc ) {
        if ( ! handler->event_epandesc( userdata, channel, channel_page, panid, addr, lqi, pairid ) ) {
            return false;
        }
    }

    return true;
}

static bool parse_erxudp( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    if ( ! read_string( fd, " " ) ) {
        return false;
    }

    char sender[64] = {};
    if ( ! read_string_to( fd, sender, sizeof( sender ), " " ) ) {
        return false;
    }
    char receiver[64] = {};
    if ( ! read_string_to( fd, receiver, sizeof( receiver ), " " ) ) {
        return false;
    }
    char sender_port[8] = {};
    if ( ! read_string_to( fd, sender_port, sizeof( sender_port ), " " ) ) {
        return false;
    }
    char receiver_port[8] = {};
    if ( ! read_string_to( fd, receiver_port, sizeof( receiver_port ), " " ) ) {
        return false;
    }
    char sender_addr[32] = {};
    if ( ! read_string_to( fd, sender_addr, sizeof( sender_addr ), " " ) ) {
        return false;
    }
    char secured_string[4] = {};
    if ( ! read_string_to( fd, secured_string, sizeof( secured_string ), " " ) ) {
        return false;
    }
    const long secured = strtol( secured_string, NULL, 10 );

    char datalen[8] = {};
    if ( ! read_string_to( fd, datalen, sizeof( datalen ), " " ) ) {
        return false;
    }

    const size_t size = strtol( datalen, NULL, 16 );
    if ( size == 0 ) {
        return false;
    }

    uint8_t * const bytes = malloc( size );
    if ( read( fd, bytes, size ) != size ) {
        return false;
    }

    if ( ! handler->event_erxudp ) {
        return false;
    }

    if ( ! handler->event_erxudp( userdata, sender, receiver, sender_port, receiver_port, sender_addr, secured == 1, size, bytes ) ) {
        return false;
    }

    return true;
}

bool bp35a1_command( const int fd, const char * const string )
{
    return bp35a1_write( fd, string, strlen( string )-1 );
}

bool bp35a1_response( const int fd, const struct bp35a1_handler * const handler, void *userdata )
{
    uint8_t buffer[16] = {};
    const size_t buffer_size = sizeof( buffer );
    size_t i = 0;
    for ( ; i < buffer_size; i++ ) {
        const ssize_t n = read( fd, buffer+i, 1 );
        if ( n == -1 ) {
            return false;
        } else if ( n == 0 ) {
            break;
        }
        static struct {
            const char *name;
            bool (*parse)( const int fd, const struct bp35a1_handler * const handler, void *userdata );
        } commands[] = {
            { "SKVER", parse_skver },
            { "EVER", parse_ever },
            { "SKINFO", parse_skinfo },
            { "EINFO", parse_einfo },
            { "SKSCAN", parse_skscan },
            { "SKSETPWD", parse_sksetpwd },
            { "SKSETRBID", parse_sksetrbid },
            { "SKSREG", parse_sksreg },
            { "SKLL64", parse_skll64 },
            { "SKJOIN", parse_skjoin },
            { "EVENT 1F", parse_event_1f },
            { "EVENT 20", parse_event_20 },
            { "EVENT 21", parse_event_21 },
            { "EVENT 22", parse_event_22 },
            { "EVENT 25", parse_event_25 },
            { "EEDSCAN", parse_eedscan },
            { "EPANDESC", parse_epandesc },
            { "ERXUDP", parse_erxudp },
            { " ", parse_whitespace },
            { "\r\n", parse_whitespace },
        };
        static const size_t command_size = sizeof( commands ) / sizeof( commands[0] );
        for ( size_t j = 0; j < command_size; j++ ) {
            if ( strcmp( (const char *)buffer, commands[j].name ) == 0 ) {
                return commands[j].parse( fd, handler, userdata );
            }
        }
    }
    if ( ! handler->unknown ) {
        return false;
    }
    return handler->unknown( userdata, buffer, i );
}
