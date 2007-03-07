#ifndef _FUPPES_H
#define _FUPPES_H

#ifdef __cplusplus
extern "C" {
#endif

#define FUPPES_TRUE  0
#define FUPPES_FALSE 1

int fuppes_init();
int fuppes_start();
int fuppes_stop();
int fuppes_cleanup();


#ifdef __cplusplus
}
#endif
#endif // _FUPPES_H
