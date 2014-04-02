#ifndef __S_DEBUG_H__
#define __S_DEBUG_H__

#define DBG(fmt, args...) do { fprintf(stdout, fmt, ##args ); } while(0)
#define ERR(fmt, args...) do { fprintf(stderr, fmt, ##args); } while(0)

#endif //__S_DEBUG_H__
