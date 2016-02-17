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
	VGImage bufferImage;
	bool bufferSaved;
	int width, height, borderWidth, centerX, centerY;
	int bottomLeftX, bottomLeftY, rectWidth, rectHeight;
	int cornerRadius;
	float borderColor[4];
	float borderColorAlpha;
	float backgroundColor[4];
	float backgroundColorAlpha;
	int buttonPadding;
	int numButtons;
	int buttonWidth, buttonHeight;
	bool isHorizontal;
	int pressDebounce;

	// Menu Title
	bool titled;
	string title;
	float titleColor[4];
	float titleColorAlpha;
	int titleFontSize, titlePercentHeight;

	// Scrollable menu
	bool scrollable;
	int scrollItems, totalItems;
	bool prevButton;
	string prevButtonText;
	bool nextButton;
	string nextButtonText;
	int scrollButtonPercentHeight;
	int numPages;
	int currentPage;
	Button* previousBtn;
	Button* nextBtn;
	int topMenuItemIndex;
	int menuItemsRemaining;
	int activeButtons;

	// Hide and fade
	bool hideable;
	int hideDeltaX;
	int hideDeltaY;
	int hideFade;
	int hideDuration;
	bool hidden;

	// Button configuration properties to be used in button creation
	bool configureButtons;
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

	// Selection
	bool* buttonSelectStates;
	string menuSelectMode;
	int timedSelectDuration;
	uint64_t timedSelectionStart;
	uint64_t timedSelectionEnd;

	uint64_t prevBtnSelectionStart;
	uint64_t prevBtnSelectionEnd;
	uint64_t nextBtnSelectionStart;
	uint64_t nextBtnSelectionEnd;

	void configure(string);					// Menu configuration
	int getVectorIndex(string);
	vector<Button> menuButtons;

public:
	string menuIdentifier;
	string menuType;
	Menu(int, int, int, int, string);		// Menu Constructor: center X, center Y, width, height, identifier
	
	void update(touch_t);					// Menu update function: draws menu & updates buttons with provided touch data
	bool isButtonPressed(string);			// Checks menu pressed states to return the press state of button with the provided name
	bool isButtonSelected(string);			// Checks menu select states to return the select state of button with the provided name
	void selectButton(string);				// Select the button with the provided name
	void deselectButton(string);			// Deselect the button with the provided name

	string getPressedButtonName(void);		// Return the name of the pressed button
	string getSelectedButtonName(void);		// Return the name of the first selected button

	void hide(void);						// Hide (and or fade) the menu
	void unhide(void);						// Un-hide (and or un-fade) the menu
	bool isHidden(void);					// Get menu hide state
	~Menu(void);							// Menu destructor (destroys vgImage)
	//Menu(const Menu &);

};

#endif
