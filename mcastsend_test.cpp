#include "udpmcastsender.h"
#include "spells.h"

int main(int argc, char **argv)
{
    std::vector<InetInterface> result;

    result = InetInterface::getAllInterfaces();

    std::vector<InetInterface>::iterator it;

    for(it=result.begin(); it!=result.end(); ++it) {
        UDPMCastSender sender;
        spell_t spell;
        ssize_t ret;
        int iret;

        spell.version = SPELL_VERSION;
        spell.spell = SPELL_APARECIUM;

        printf("interface %s, IP %s\n", it->name, it->ip);
        iret = sender.Init(*it, "239.139.139.139", 1394, false);

        printf("sender init %d\n", iret);

        ret = sender.Send((void *)&spell, sizeof(spell));

        printf("sending result %ld\n", ret);
    }

}

