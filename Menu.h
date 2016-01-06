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
	int timedSelectDuration;
	uint64_t timedSelectionStart;
	uint64_t timedSelectionEnd;


	void configure(string);
	int getVectorIndex(string);
	vector<Button> menuButtons;

public:
	Menu(int, int, int, int, string);		// Menu Constructor: center X, center Y, width, height, identifier
	
	void update(touch_t);
	bool isButtonPressed(string);
	bool isButtonSelected(string);
	void selectButton(string);
	void deselectButton(string);

	string getPressedButtonName(void);

	void hide(void);
	void unhide(void);
	bool isHidden(void);

};

#endif
