#ifndef __SUMMONER_SPELL_H__
#define __SUMMONER_SPELL_H__

#define SPELL_MULTICAST_IP "239.139.139.139"
#define SPELL_MULTICAST_PORT (1394)

#define SPELL_VERSION (139)

enum {
    SPELL_INVALID = 0,
    SPELL_APARECIUM,
    SPELL_APARECIUM_REPLY,
};

#pragma pack(push, 1)

typedef struct _spell_t {
    unsigned char version;
    unsigned char spell;
    unsigned char data[512];
} spell_t;

#pragma pack(pop)

#endif //__SUMMONER_SPELL_H__
