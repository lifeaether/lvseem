//
//  bp35a1.h
//  lvseem
//
//  Created by lifeaether on 2021/04/29.
//

#ifndef bp35a1_h
#define bp35a1_h

#include <stdint.h>
#include <stdbool.h>

struct bp35a1_handler {
    bool (*event_ever)( void * const userdata, const char * const version );
    bool (*event_einfo)( void * const userdata, const char * const ipaddr, const char * const addr64, const char * const channel, const char * const panid, const char * const addr16 );
    bool (*event_1f)( void * const userdata, const char * const sender );
    bool (*event_eedscan)( void * const userdata, const char * const channel_rssi );
};

bool bp35a1_command( const int fd, const char *string );
bool bp35a1_response( const int fd, const struct bp35a1_handler * const handler, void *userdata );

#endif /* bp35a1_h */
