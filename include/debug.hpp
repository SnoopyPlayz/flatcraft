#pragma once
#include <string>

class Debug {
	public:
		void draw();
		void addMessage(std::string mesage);
	private:
		bool enabled = false;
};

extern Debug debug;
