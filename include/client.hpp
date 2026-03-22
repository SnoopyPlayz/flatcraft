#pragma once
#include <cstdint>
#include <stop_token>

bool createClient(const char* host, uint16_t port);
void drawClients();
void updateClient(std::stop_token st);
void updateClients();
