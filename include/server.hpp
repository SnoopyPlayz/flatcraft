#pragma once
#include <cstdint>
#include <stop_token>

bool hostServer(uint16_t port);
void updateServer(std::stop_token st);
