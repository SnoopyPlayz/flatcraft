#include "debug.hpp"
#include "rayUtils.hpp"
#include "raylib.h"
#include "stdio.h"
#include <string>
#include <vector>

Debug debug;

std::vector<std::string> messages;

void Debug::draw() {
	if (IsKeyPressed(KEY_F3))
		enabled = !enabled;
	if (!enabled){
		return;
	}

	const int fontSize = 30;
	float topBarHeight = fontSize;
	Color color = WHITE;

#ifdef NDEBUG
	drawTextSDF("F3 to disable Debug menu. RELEASE ver", 0, 0, fontSize, WHITE);
#else
	drawTextSDF("F3 to disable Debug menu. DEBUG ver", 0, 0, fontSize, WHITE);
#endif

	for (const std::string &s : messages) {
		std::string cText = "";

		for (size_t i {}; i < s.size(); i++) {
			char c = s[i];

			// edge case % at end
			if (c == '%' && i + 1 >= s.size()) {
				break;
			}

			// removal of char after %
			if (i > 0 && s[i - 1] == '%') {
				continue;
			}

			// change color
			if (c == '%') {
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

			float width = MeasureTextEx(fontSDF, cText.c_str(), fontSize, 0).x;
			drawTextSDF(std::string(1, c).c_str(), width + 1, topBarHeight + 1, fontSize, GRAY);
			drawTextSDF(std::string(1, c).c_str(), width, topBarHeight, fontSize, ColorTint(color, WHITE));
			cText += c;
		}

		color = WHITE;
		topBarHeight += fontSize;
	}
	messages.clear();
}

void Debug::addMessage(std::string message) {
	if (!enabled)
		return;
	messages.push_back(message);
}
