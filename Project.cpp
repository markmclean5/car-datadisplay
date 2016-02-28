using namespace std;
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "VG/openvg.h"	//
#include "VG/vgu.h"		//
#include "EGL/egl.h"	// EGL for pbuffers
#include "fontinfo.h"	// Openvg
#include "shapes.h"		//
#include <stdlib.h>		//
#include "Gauge.h"	// Gauge class
#include "DataStream.h"	// DataStream class
#include <linux/input.h>
#include <fcntl.h>
#include <pthread.h>
#include "avengeance.inc"	// *** Label Fonts
#include "digits.inc"		// *** Label Fonts
#include <bcm2835.h>
#include <vector>

// Project Classes & utilities
#include "touchscreen.h"
#include "project.h"
#include "Button.h"
#include "TextView.h"
#include "Menu.h"
#include "Serial.h"
#include "PID.h"
#include <algorithm>



// Loop time
uint64_t loopTime;
touch_t threadTouch, loopTouch;
int quitState = 0;

VGImage BackgroundImage;

// Prototypes
void setupGraphics(int*,int*);

void DisplayObjectManager(std::vector<Button>&, std::vector<Gauge>&, std::vector<PID>&, std::vector<Menu>&);


//Dashboard mode vectors
vector<Button> DASHBOARD_HotButtons;
vector<Gauge> DASHBOARD_Gauges;
vector<PID> DASHBOARD_PIDs;
vector<Menu> DASHBOARD_Menus;




// Color definitions (float r, float g, float b, float alpha)
float black[] = {0,0,0,1};
float green[] = {0,1,0,1};
float red[] = {1,0,0,1};
float blue[] = {0,0,1,1};
float white[] = {1,1,1,1};
float translucentBlack[] = {0,0,0,0.5};
float gray[] = {0.43,0.43,0.43,1};
float sendcolor[] = {1.0, 0.4, 0.4, 1.0};
float receivecolor[] = {0.4, 0.69, 1.0, 1.0};

// main()
int main() {
	int width, height;					// display width & height
	setupGraphics(&width, &height);		// Initialize display

	if (!bcm2835_init())
		return 1;
	if (touchinit(width, height) != 0) {
		fprintf(stderr, "Unable to initialize the mouse\n");
		exit(1);
	}

	DASHBOARD_Menus.reserve(4);
	DASHBOARD_HotButtons.reserve(4);
	DASHBOARD_PIDs.reserve(4);
	DASHBOARD_Gauges.reserve(4);

	// Draw background
	Image(0, 0, 800, 480, "/home/pi/openvg/client/wallpaper.jpg");
	BackgroundImage = vgCreateImage(VG_sABGR_8888, 800, 480, VG_IMAGE_QUALITY_BETTER);
	vgGetPixels(BackgroundImage, 0, 0, 0, 0, 800, 480);
	

	Menu Mode1Menu(width/2, height-30, width-20, 50, "Mode1Menu");
	Mode1Menu.touchEnable();
	Mode1Menu.selectButton("m1");
	int currentMode = 0;

	Serial ELMSerial("/dev/ELM327", B38400);
	ELMSerial.serialWrite("ATZ");
	ELMSerial.setEndChar('>');
	ELMSerial.setReadTimeout(5000);

	//////////////////////////////////////
	// Main Execution Loop
	///////////////////////////////////////
	while(1) {
		loopTouch = threadTouch;											// Get touch for loop
		loopTime = bcm2835_st_read();										// Get time for loop
		vgSetPixels(0, 0, BackgroundImage, 0, 0, 800, 480);					// Draw background image
		Mode1Menu.update(loopTouch);										// Update mode menu

		if(Mode1Menu.isButtonPressed("m1")) Mode1Menu.selectButton("m1");	// Mode menu selection
		if(Mode1Menu.isButtonPressed("m2")) Mode1Menu.selectButton("m2");
		if(Mode1Menu.isButtonPressed("m3")) Mode1Menu.selectButton("m3");
		if(Mode1Menu.isButtonPressed("m4")) Mode1Menu.selectButton("m4");
		if(Mode1Menu.isButtonPressed("m5")) Mode1Menu.selectButton("m5");

		

		//////////////////////////////////////
		// Mode 1 - DASHBOARD
		//////////////////////////////////////
		if(Mode1Menu.isButtonSelected("m1")) {
			
			TextView SerialViewer(width/3 - width/6, height/2, width/3, 360, "SerialViewer");
			int numPIDs = 0;

			// Set up iterator for PID vector
			bool waitingOnData = false;
			std::vector<PID>::iterator p = DASHBOARD_PIDs.begin();

			bool menuVisible = false;

			ELMSerial.serialWrite("ATZ");
			string serialData = "";

			int newObjectCenterX = width/2;
			int newObjectCenterY = height/2;


			int gaugeCenterX = width/2;
			int gaugeCenterY = height/2-30;
			int gaugeRadius = height/2 - 80;

			while(1) {


				if(DASHBOARD_PIDs.size() != numPIDs) {
					numPIDs = DASHBOARD_PIDs.size();
					p = DASHBOARD_PIDs.begin();
				}
				
				loopTime = bcm2835_st_read();
				loopTouch = threadTouch;
				serialData = ELMSerial.serialReadUntil();
				// Request data if: no pending request
				if(!waitingOnData && DASHBOARD_PIDs.size()>0) {
					ELMSerial.serialWrite((p)->getCommand());
					waitingOnData = true;

				}
				else if(waitingOnData && !serialData.empty() && DASHBOARD_PIDs.size()>0) {
					waitingOnData = false;
					(p)->update(serialData, loopTime);
					SerialViewer.addNewLine((p)->getCommand() + " - " + to_string((p)->getRawUpdateRate()) + " Hz");

					if(p<DASHBOARD_PIDs.end()) p++;
					if(p == DASHBOARD_PIDs.end()) p = DASHBOARD_PIDs.begin();
				}
		

				vgSetPixels(0, 0, BackgroundImage, 0, 0, 800, 480);
				Mode1Menu.update(loopTouch);


				// Update all hot buttons
				for (std::vector<Button>::iterator it = DASHBOARD_HotButtons.begin(); it != DASHBOARD_HotButtons.end(); it++) {
					(it)->updateTouch(loopTouch);
					(it)->update();
				}

				

				// Update all gauges
				for (std::vector<Gauge>::iterator it = DASHBOARD_Gauges.begin(); it != DASHBOARD_Gauges.end(); it++) {
					(it)->updateTouch(loopTouch);
					string name = (it)->getPIDLink();
					cout << "Current Gauge PID Link: " << name << endl;
					auto CurrentGaugePID_It = find_if(DASHBOARD_PIDs.begin(), DASHBOARD_PIDs.end(), [&name](PID& obj) {return obj.getCommand().compare(name) == 0;});
					if(CurrentGaugePID_It == DASHBOARD_PIDs.end()) {
						cout << "PID Not found" << endl;
						(it)->update(0,"RPM");
					} 
					else {			
						(it)->update((CurrentGaugePID_It)->getRawDatum(), (CurrentGaugePID_It)->getEngUnits());
					}
					
					
				}

				SerialViewer.update();
				// Update all menus
				for (std::vector<Menu>::iterator it = DASHBOARD_Menus.begin(); it != DASHBOARD_Menus.end(); it++) {
					(it)->update(loopTouch);
				}
				// Run DisplayObjectManager (current page dashboard hotbuttons, display objects, and PIDs)
				DisplayObjectManager(DASHBOARD_HotButtons, DASHBOARD_Gauges, DASHBOARD_PIDs, DASHBOARD_Menus);


				if(Mode1Menu.isButtonPressed("m2")) {
					Mode1Menu.selectButton("m2");
					break;
				}
				if(Mode1Menu.isButtonPressed("m3")) {
					Mode1Menu.selectButton("m3");
					break;
				}
				if(Mode1Menu.isButtonPressed("m4")) {
					Mode1Menu.selectButton("m4");
					break;
				}
				if(Mode1Menu.isButtonPressed("m5")) {
					Mode1Menu.selectButton("m5");
					break;
				}

				End();
			}

		}

		//////////////////////////////////////
		// Mode 5 - PID Support Check
		//////////////////////////////////////
		if(Mode1Menu.isButtonSelected("m5")) {
			Menu PIDSupportMenu(width/6, height/2, width/3 - 20, 360, "PIDSupport");
			TextView PIDList(width/2, height/2, width/3-20, 360, "PIDList");

			Menu SupportedPIDMenu(width - width/6, height/2, width/3 - 20, 360, "SupportedPIDMenu");
			
			vector<PID> SupportPIDs;
			vector<PID> SupportedPIDs;

			
			
			while(1) {
				loopTime = bcm2835_st_read();
				loopTouch = threadTouch;
				vgSetPixels(0, 0, BackgroundImage, 0, 0, 800, 480);
				Mode1Menu.update(loopTouch);
				SupportedPIDMenu.update(loopTouch);

				PIDSupportMenu.update(loopTouch);
				string menuButton = PIDSupportMenu.getPressedButtonName();
				if(!menuButton.empty()) {
					PIDSupportMenu.selectButton(menuButton);
				}

				if(!PIDSupportMenu.getSelectedButtonName().empty() && SupportPIDs.size() == 0) {
					cout << "requesting supported pids - " << PIDSupportMenu.getSelectedButtonName() << endl;
					SupportPIDs.emplace_back(PIDSupportMenu.getSelectedButtonName());

					SupportPIDs.back().update("41 00 00 00 FF FF", loopTime);

					for(int i = 0; i < SupportPIDs.back().getNumBits(); i++) {
						cout << i << " - Name "<< SupportPIDs.back().getBitPositionName(i) << " - State " << SupportPIDs.back().getBitPositionState(i) << endl;
						if(SupportPIDs.back().getBitPositionValue(i)) SupportedPIDMenu.addItem(SupportPIDs.back().getBitPositionName(i), SupportPIDs.back().getBitPositionName(i));
					}
				}
					
				PIDList.update();



				
				if(Mode1Menu.isButtonPressed("m1")) {
					Mode1Menu.selectButton("m1");
					break;
				}
				if(Mode1Menu.isButtonPressed("m2")) {
					Mode1Menu.selectButton("m2");
					break;
				}
				if(Mode1Menu.isButtonPressed("m3")) {
					Mode1Menu.selectButton("m3");
					break;
				}
				if(Mode1Menu.isButtonPressed("m4")) {
					Mode1Menu.selectButton("m4");
					break;
				}
				End();
			}

		}
		// Write screen buffer to screen
		End();
	}
}




void DisplayObjectManager(std::vector<Button>& HotButtons, std::vector<Gauge>& Gauges, std::vector<PID>& PIDs, std::vector<Menu>& Menus){

	int width = 800;
	int height = 480;
	// Create & touch enable hotbuttons if no buttons or gauges exist
	if(HotButtons.size() == 0 && Gauges.size() == 0) {
		HotButtons.emplace_back(800/6, (480/2) - 30, 80, 80, "hotBtn1");
		HotButtons.back().touchEnable();
		HotButtons.emplace_back(800/2, (480/2) - 30, 80, 80, "hotBtn2");
		HotButtons.back().touchEnable();
		HotButtons.emplace_back(800 - 800/6, (480/2) - 30, 80, 80, "hotBtn3");
		HotButtons.back().touchEnable();
	}


	std::vector<Button>::iterator selectedHotbutton_It = HotButtons.end();

	// Handle all hot button logic here
	for (std::vector<Button>::iterator currentHotButton = HotButtons.begin(); currentHotButton != HotButtons.end(); currentHotButton++) {
		if(currentHotButton->isPressed()) currentHotButton->select();

		// When hotbutton is released, disable touch on hotbuttons & display objects
		// Also create parameter selection menus
		if(currentHotButton->isReleased()) {
			for (std::vector<Button>::iterator hb = HotButtons.begin(); hb != HotButtons.end(); hb++)
				hb->touchDisable();
			for (std::vector<Gauge>::iterator g = Gauges.begin(); g != Gauges.end(); g++)
				g->touchDisable();
			Menus.emplace_back(width/6, (height/2)-30, (width/3) - 10, height-70, "HOTBUTTON_GroupMenu");
			string defaultGroup = "g1";
			Menus.back().selectButton(defaultGroup);
			Menus.emplace_back(width-width/6, (height/2)-30, (width/3)- 10, height-70, "HOTBUTTON_DisplayObjectMenu");
			cout << "default group: " << defaultGroup << endl;
			Menus.emplace_back(width/2, (height/2)-30, (width/3)- 10, height-70, defaultGroup);
		}

		// If hotbutton is selected - set selected button iterator
		if(currentHotButton->isSelected()) selectedHotbutton_It = currentHotButton;
	}

	// Create menus if menus are not present
	if(Menus.size() != 0) {

		string name = "HOTBUTTON_GroupMenu";
		auto HOTBUTTON_GroupMenu_It = find_if(Menus.begin(), Menus.end(), [&name](const Menu& obj) {return obj.menuIdentifier.compare(name) == 0;});
		name = "HOTBUTTON_DisplayObjectMenu";
		auto HOTBUTTON_DisplayObjectMenu_It = find_if(Menus.begin(), Menus.end(), [&name](const Menu& obj) {return obj.menuIdentifier.compare(name) == 0;});
		string type = "ParameterMenu";
		auto HOTBUTTON_ParameterMenu_It = find_if(Menus.begin(), Menus.end(), [&type](const Menu& obj) {return obj.menuType.compare(type) == 0;});


		for (std::vector<Menu>::iterator currentMenu = Menus.begin(); currentMenu != Menus.end(); currentMenu++) {
			string pressedButtonName = currentMenu->getPressedButtonName();
			if(!pressedButtonName.empty()) currentMenu->selectButton(pressedButtonName);
		}
		
		if(HOTBUTTON_GroupMenu_It->getSelectedButtonName().compare(HOTBUTTON_ParameterMenu_It->menuIdentifier) != 0) {
			cout << "changing parameter menu to: "<<  HOTBUTTON_GroupMenu_It->getSelectedButtonName() << endl; 
			Menus.erase(HOTBUTTON_ParameterMenu_It);
			Menus.emplace_back(width/2, (height/2)-30, (width/3)- 10, height-70, HOTBUTTON_GroupMenu_It->getSelectedButtonName());
			type = HOTBUTTON_GroupMenu_It->getSelectedButtonName();
			HOTBUTTON_ParameterMenu_It = find_if(Menus.begin(), Menus.end(), [&type](const Menu& obj) {return obj.menuType.compare(type) == 0;});
		}

		


		// All selections made, add item and PID
		if(	!HOTBUTTON_GroupMenu_It->getSelectedButtonName().empty()
			&& !HOTBUTTON_ParameterMenu_It->getSelectedButtonName().empty()
			&& !HOTBUTTON_DisplayObjectMenu_It->getSelectedButtonName().empty()) {

			cout << "selected group: " << HOTBUTTON_GroupMenu_It->getSelectedButtonName() << endl;
			cout << "selected parameter: " << HOTBUTTON_ParameterMenu_It->getSelectedButtonName() << endl;
			cout << "selected display type:" << HOTBUTTON_DisplayObjectMenu_It->getSelectedButtonName() << endl;

			if(HOTBUTTON_DisplayObjectMenu_It->getSelectedButtonName().compare("Gauge") == 0) {
				cout << "creating a gauge!!" << endl;
				PIDs.emplace_back(HOTBUTTON_ParameterMenu_It->getSelectedButtonName());
				Gauges.emplace_back(selectedHotbutton_It->getDOPosX(), selectedHotbutton_It->getDOPosY(), width/6, HOTBUTTON_ParameterMenu_It->getSelectedButtonName().append("Gauge"));
				Gauges.back().draw();
				Gauges.back().touchEnable();
				// Erase menus in reverse creation order to prevent invalid iterators		
				Menus.erase(HOTBUTTON_ParameterMenu_It);
				Menus.erase(HOTBUTTON_DisplayObjectMenu_It);
				Menus.erase(HOTBUTTON_GroupMenu_It);
				
				// Hide the selected hot button and re-enable touch on all other visible buttons
				for (std::vector<Button>::iterator currentHotButton = HotButtons.begin(); currentHotButton != HotButtons.end(); currentHotButton++) {
					if(currentHotButton == selectedHotbutton_It) {
						currentHotButton->deselect();
						currentHotButton->setInvisible();
					}
					else
						if(currentHotButton->isVisible())
							currentHotButton->touchEnable();
				}
				// Re-enable touch on other display objects
				for (std::vector<Gauge>::iterator g = Gauges.begin(); g != Gauges.end(); g++)
					g->touchEnable();

			}
		}
	}


	// Delete a gauge & corresponding PID when it is pressed and released
	for (std::vector<Gauge>::iterator currentGauge = Gauges.begin(); currentGauge != Gauges.end(); currentGauge++) {
		if(currentGauge->isReleased()) {
			int gX = currentGauge->getDOPosX();
			int gY = currentGauge->getDOPosY();
			string PIDLink = currentGauge->getPIDLink();
			Gauges.erase(currentGauge);
			for (std::vector<Button>::iterator currentHotButton = HotButtons.begin(); currentHotButton != HotButtons.end(); currentHotButton++) {
				if(currentHotButton->getDOPosX() == gX && currentHotButton->getDOPosY() == gY) {
					currentHotButton->setVisible();
					currentHotButton->touchEnable();
				}
			}
			// Erase corresponding PID
			auto PID_It = find_if(PIDs.begin(), PIDs.end(), [&PIDLink](const PID& obj) {return obj.command.compare(PIDLink) == 0;});
			if(PID_It != PIDs.end())
			PIDs.erase(PID_It);
			break;
		}
	}

}



// setupGraphics()
void setupGraphics(int* widthPtr, int* heightPtr) {	
	init(widthPtr,heightPtr);
	Start(*widthPtr, *heightPtr);		//Set up graphics, start picture
	Background(0,0,0);
}

