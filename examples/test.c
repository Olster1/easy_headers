#define EASY_ANIMATION_2D_IMPLEMENTATION 1
#include "easy_animation_2d.h"
#include "easy_string_utf8.h"

int main(char **args, int argc) {

	EasyAnimation_Controller c;
	easyAnimation_initController(&c);

	printf("%d\n", easyAnimation_isControllerEmpty(&c));

	return 0;
}