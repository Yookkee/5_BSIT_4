#ifndef PROTOCOL_H
#define PROTOCOL_H


enum COMMANDS { VERSION = 1, TICKS, MEMORY, DISKS, OWNER, ACL_ACE, TIME, HELLO = 95, HELP = 90, QUIT = 99 };
#define REQUEST_BIT 128
#define RESPONSE_BIT 0


#endif