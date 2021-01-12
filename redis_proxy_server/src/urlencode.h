#ifndef __URLENCODE_H_
#define __URLENCODE_H_

/* Allow C++ programs to link to this. */
#ifdef __cplusplus
extern "C" {
#endif

char* url_decode( const char* str );
char* url_encode( const char* str );

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif
