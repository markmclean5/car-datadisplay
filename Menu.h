#ifndef MENU_H
#define MENU_H

#include "TouchableObject.h"
#include "Button.h"
#include <vector>

/********************************/
/*			Menu Class			*/
/********************************/

using namespace std;

class Menu : public TouchableObject {
private:
	string menuIdentifier;

	int width, height;
	int borderWidth;
	int centerX, centerY;
	int bottomLeftX, bottomLeftY;
	int rectWidth, rectHeight;
	int cornerRadius;
	float borderColor[4];
	float borderColorAlpha;
	float backgroundColor[4];
	float backgroundColorAlpha;

	int buttonPadding;
	int numButtons;
	bool isHorizontal;

	int buttonWidth, buttonHeight;

	void configure(string);


public:
	Menu(int, int, int, int, string);		// Menu Constructor: center X, center Y, width, height, identifier
	vector<Button> menuButtons;
	void update(touch_t);
};

#endif
