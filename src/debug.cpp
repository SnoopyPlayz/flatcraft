#include "debug.hpp"
#include "raylib.h"
#include "stdio.h"
#include <string>
#include <vector>

Debug debug;

std::vector<std::string> messages;


void Debug::draw(){
	if (IsKeyPressed(KEY_F3)){
		enabled = !enabled;
	}
	if (!enabled) return;

	const int fontSize = 20;

	int topBarHeight = fontSize;
	DrawText("Debug menu F3 to disable", 0, 0, fontSize, WHITE);
	Color color = WHITE;

	for (std::string s : messages) {
		std::string cText = "";
		for (int i{0}; i < s.size(); i++){
			char c = s[i];

			if (c == '%' && i + 1 >= s.size()) { 
				break;
			}

			if (c == '%'){
				if (s[i + 1] == 'B') {
					color = BLUE;
				} else if (s[i + 1] == 'G') {
					color = GREEN;
				} else if (s[i + 1] == 'R') {
					color = RED;
				} else if (s[i + 1] == 'W') {
					color = WHITE;
				}

				continue;
			}

			int width = MeasureText(cText.c_str(), fontSize);
			DrawText(std::string(1, c).c_str(), width + 1, topBarHeight + 1, fontSize, GRAY);
			DrawText(std::string(1, c).c_str(), width, topBarHeight, fontSize, ColorTint(color, WHITE));
			cText += c;
		}

		color = WHITE;
		topBarHeight += fontSize;
	}
	messages.clear();
}



void Debug::addMessage(std::string message){
	if (!enabled) return;
	messages.push_back(message);
}
