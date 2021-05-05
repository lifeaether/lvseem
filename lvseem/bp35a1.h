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

bool bp35a1_write( const int fd, const void * const bytes, const size_t size );

struct bp35a1_handler {
    bool (*event_ever)( void * const userdata, const char * const version );
};

bool bp35a1_parse( const int fd, const struct bp35a1_handler * const handler, void *userdata );

#endif /* bp35a1_h */
