/*
 * Copyright (C) 2025 M. Wykpis, A. Szwaja, P. Kubicki, S. Szulc, K. Socha
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef FDS_LISTNER_HPP
#define FDS_LISTNER_HPP

#include<functional>

void listen_on_fds(std::function<void(int, char*, int)> func);

#endif // FDS_LISTNER_HPP
