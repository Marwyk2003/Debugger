#ifndef FDS_LISTNER_HPP
#define FDS_LISTNER_HPP

#include<functional>

void listen_on_fds(std::function<void(int, char*, int)> func);

#endif // FDS_LISTNER_HPP
