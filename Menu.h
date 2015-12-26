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

<<<<<<< HEAD

=======
>>>>>>> origin/master
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
<<<<<<< HEAD
=======

>>>>>>> origin/master
	int buttonPadding;
	int numButtons;
	bool isHorizontal;

<<<<<<< HEAD
	int pressDebounce;

	bool hideable;
	int hideDeltaX;
	int hideDeltaY;
	int hideFade;
	int hideDuration;
	int buttonWidth, buttonHeight;
	bool hidden;
	bool configureButtons;

	// Button configuration properties to be used in button creatio
	float buttonBackgroundColor[4];
	float buttonSelectedBackgroundColor[4];
	float buttonBorderColor[4];
	int buttonBorderWidth;
	float buttonSelectedBorderColor[4];
	int buttonSelectedBorderWidth;
	float buttonTextColor[4];
	float buttonSelectedTextColor[4];
	char buttonTextVertAlign;
	int buttonCornerRadius;

	string* buttonNames;
	string* buttonCfgText;
	bool* buttonSelectStates;
	string menuSelectMode;


	void configure(string);
	int getVectorIndex(string);
=======
	int buttonWidth, buttonHeight;

	void configure(string);
>>>>>>> origin/master


public:
	Menu(int, int, int, int, string);		// Menu Constructor: center X, center Y, width, height, identifier
	vector<Button> menuButtons;
	void update(touch_t);
<<<<<<< HEAD
	bool isButtonPressed(string);
	bool isButtonSelected(string);
	void selectButton(string);
	void deselectButton(string);

	void hide(void);
	void unhide(void);
	bool isHidden(void);

=======
>>>>>>> origin/master
};

#endif
