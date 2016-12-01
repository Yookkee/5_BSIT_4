#ifndef INFO_COLLECTOR_H
#define INFO_COLLECTOR_H

#include <string>

std::string do_get_os_version();
std::string do_get_current_time();
std::string do_get_ticks();
std::string do_get_memory_info();
std::string do_get_disks_info();
std::string do_get_owner(const char * str);
std::string do_get_acl(const char * str);

#endif