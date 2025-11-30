#include <stdio.h>
#include <raylib.h>

int main(){
	InitWindow(100, 100, "test");
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();

		EndDrawing();
	}
	printf("hello");
	return 0;
}
