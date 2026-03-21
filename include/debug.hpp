#pragma once
#include <string>

// Enable with F3. color codes: %R (red), %G (green), %B (blue), %W (white)

class Debug {
	public:
		void draw();
		void addMessage(std::string mesage);
		bool enabled = false;
};

extern Debug debug;
