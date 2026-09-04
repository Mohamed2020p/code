/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */
#ifndef SYMBOL_FINDER
#define SYMBOL_FINDER

#include <unistd.h>

extern int find_name(pid_t pid, const char *name,const  char *libn, unsigned long *addr);
extern int find_libbase(pid_t pid, const char *libn, unsigned long *addr);
#endif