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


// Loop time
uint64_t loopTime;
touch_t threadTouch, loopTouch;
int quitState = 0;

VGImage BackgroundImage;

// Prototypes
void setupGraphics(int*,int*);

//Dashboard mode vectors
vector<Button> DASHBOARD_HotButtons;
vector<Gauge> DASHBOARD_Gauges;
vector<PID> DASHBOARD_PIDs;

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



	///////////////////////////////////////
	// Dashboard Objects
	///////////////////////////////////////
	// Hot Buttons
	DASHBOARD_HotButtons.emplace_back(width/6, (height/2) - 30, 80, 80, "hotBtn1");
	DASHBOARD_HotButtons.emplace_back(width/2, (height/2) - 30, 80, 80, "hotBtn2");
	DASHBOARD_HotButtons.emplace_back(width - width/6, (height/2) - 30, 80, 80, "hotBtn3");
	// Group Menu - Select group of parameters
	Menu HOTBUTTON_GroupMenu(width/6, (height/2)-30, (width/3) - 10, height-70, "HOTBUTTON_GroupMenu");
	string groupName = "g1";
	HOTBUTTON_GroupMenu.selectButton(groupName);
	// Menu vector for Parameter Menu (of current selected group)
	vector<Menu> HOTBUTTON_ParameterMenus;
	HOTBUTTON_ParameterMenus.emplace_back(width/2, (height/2)-30, (width/3)- 10, height-70, groupName);
	// Display Object Menu (choose display method)
	Menu HOTBUTTON_DisplayObjectMenu(width-width/6, (height/2)-30, (width/3)- 10, height-70, "HOTBUTTON_DisplayObjectMenu");

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
			for (std::vector<Button>::iterator it = DASHBOARD_HotButtons.begin(); it != DASHBOARD_HotButtons.end(); it++)
				(it)->touchEnable();


			// Set up iterator for PID vector
			bool waitingOnData = false;
			std::vector<PID>::iterator p = DASHBOARD_PIDs.begin();

			bool menuVisible = false;
		

			ELMSerial.serialWrite("ATZ");
			string serialData = "";
			int gaugeCenterX = width/2;
			int gaugeCenterY = height/2;
			int gaugeRadius = height/2 - 60;

			while(1) {
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

					if(p<DASHBOARD_PIDs.end()) p++;
					if(p == DASHBOARD_PIDs.end()) p = DASHBOARD_PIDs.begin();
				}
		

				vgSetPixels(0, 0, BackgroundImage, 0, 0, 800, 480);
				Mode1Menu.update(loopTouch);

				// Manage hot buttons
				for (std::vector<Button>::iterator it = DASHBOARD_HotButtons.begin(); it != DASHBOARD_HotButtons.end(); it++) {
					(it)->updateTouch(loopTouch);
					(it)->update();
					if((it)->isPressed()) (it)->select();

					// if hot button pressed, draw the menu
					if((it)->isReleased() && !menuVisible) {
							(it)->deselect();
							menuVisible = true;
							for(std::vector<Button>::iterator btn = DASHBOARD_HotButtons.begin(); btn != DASHBOARD_HotButtons.end(); btn++)
								(btn)->touchDisable();					
					}
				}

				if(menuVisible) {
					HOTBUTTON_GroupMenu.update(loopTouch);
					string groupNamePressed = "";
					groupNamePressed= HOTBUTTON_GroupMenu.getPressedButtonName();
					if(!groupNamePressed.empty()) {
						HOTBUTTON_GroupMenu.selectButton(groupNamePressed);
						if(groupNamePressed.compare(groupName) != 0) {
							groupName = groupNamePressed;

							if(HOTBUTTON_ParameterMenus.size()>0)
								HOTBUTTON_ParameterMenus.erase(HOTBUTTON_ParameterMenus.end());
							HOTBUTTON_ParameterMenus.emplace_back(width/2, (height/2)-30, (width/3)- 10, height-70, groupName);
						}
					}
					if(HOTBUTTON_ParameterMenus.size()>0) {
						for(std::vector<Menu>::iterator m = HOTBUTTON_ParameterMenus.begin(); m != HOTBUTTON_ParameterMenus.end(); m++) {
							(m)->update(loopTouch);
							string parameterPressed = "";
							parameterPressed = (m)->getPressedButtonName();
							if(!parameterPressed.empty()) {
								(m)->selectButton(parameterPressed);
							}
						}
					}
					HOTBUTTON_DisplayObjectMenu.update(loopTouch);
					string buttonName = HOTBUTTON_DisplayObjectMenu.getPressedButtonName();
					if(!buttonName.empty()) HOTBUTTON_DisplayObjectMenu.selectButton(buttonName);



					string selGroupName = HOTBUTTON_GroupMenu.getSelectedButtonName();
					string selParamName = HOTBUTTON_ParameterMenus[0].getSelectedButtonName();
					string selDispObjName = HOTBUTTON_DisplayObjectMenu.getSelectedButtonName();

					if(!selGroupName.empty() && !selParamName.empty() && !selDispObjName.empty()) {
						cout << "Selection complete!" << endl;
						cout << "Group: " << selGroupName << endl;
						cout << "Parameter: " << selParamName << endl;
						cout << "Display Object: " << selDispObjName << endl;


						menuVisible = false;
						for(std::vector<Button>::iterator btn = DASHBOARD_HotButtons.begin(); btn != DASHBOARD_HotButtons.end(); btn++)
								(btn)->touchEnable();
						HOTBUTTON_ParameterMenus[0].deselectButton(selParamName);
						HOTBUTTON_DisplayObjectMenu.deselectButton(selDispObjName);

						// Add PID to PID vector
						DASHBOARD_PIDs.emplace_back(selParamName);
						p = DASHBOARD_PIDs.begin();

						if(selDispObjName.compare("Gauge") == 0) {
							DASHBOARD_Gauges.emplace_back(gaugeCenterX, gaugeCenterY, gaugeRadius, selParamName.append("Gauge"));
							DASHBOARD_Gauges.back().draw();
							DASHBOARD_Gauges.back().touchEnable();
						}
				
					}
				}

				if(DASHBOARD_Gauges.size()>0) {
					std::vector<PID>::iterator currentPID = DASHBOARD_PIDs.begin();
					for(std::vector<Gauge>::iterator currentGauge = DASHBOARD_Gauges.begin(); currentGauge != DASHBOARD_Gauges.end(); currentGauge++) {
						(currentGauge)->update((currentPID)->getRawDatum(), "RPM");
						(currentGauge)->updateTouch(loopTouch);
						if((currentGauge)->isPressed()) {
							DASHBOARD_Gauges.erase(currentGauge);
							DASHBOARD_PIDs.erase(currentPID);
							break;
						}

						currentPID++;
					}
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
				if(Mode1Menu.isButtonPressed("m5")) {
					Mode1Menu.selectButton("m5");
					break;
				}
				End();
			}

		}

		//////////////////////////////////////
		// Mode 5 - COMM
		//////////////////////////////////////
		if(Mode1Menu.isButtonSelected("m5")) {
			TextView SerialViewer(width/4, height/2, width/2-20, 360, "SerialViewer");
			Menu SerialViewerMenu(width/4, 30, width/2-20, 50, "SerialViewerMenu");
			Menu PIDMenu = Menu(width-width/4, height/2, width/2-20, 360, "PIDMenu1");
			
			
			SerialViewerMenu.touchEnable();
			PIDMenu.touchEnable();

			while(1) {
				loopTime = bcm2835_st_read();
				loopTouch = threadTouch;
				vgSetPixels(0, 0, BackgroundImage, 0, 0, 800, 480);
				




				Mode1Menu.update(loopTouch);
				SerialViewerMenu.update(loopTouch);
				PIDMenu.update(loopTouch);

				string serialViewerCommand = SerialViewerMenu.getPressedButtonName();
				if(!serialViewerCommand.empty()) {
					SerialViewerMenu.selectButton(serialViewerCommand);
					if(serialViewerCommand.compare("Reset") == 0) {
						SerialViewer.addNewLine("ATZ", sendcolor);
						ELMSerial.serialWrite("ATZ");
					}
					if(serialViewerCommand.compare("Auto") == 0) {
						SerialViewer.addNewLine("ATSP0", sendcolor);
						ELMSerial.serialWrite("ATSP0");
					}
					if(serialViewerCommand.compare("Disp") == 0) {
						SerialViewer.addNewLine("ATDP", sendcolor);
						ELMSerial.serialWrite("ATDP");
					}
					if(serialViewerCommand.compare("Clear") == 0) {
						SerialViewer.clear();
					}
				}


				string commandString = PIDMenu.getPressedButtonName();
				if(!commandString.empty()) {
					PIDMenu.selectButton(commandString);
					SerialViewer.addNewLine(commandString, sendcolor);
					ELMSerial.serialWrite(commandString);
				}
				
				SerialViewer.update();
				string data = ELMSerial.serialReadUntil();


				if(!data.empty()) {
					cout << "Data: " << endl << data << endl;
					cout << "Data characters: " << endl;
					for(int idx=0; idx<data.size(); idx++)
						cout << (int)data[idx] << " ";
					cout << endl;
					SerialViewer.addNewLine(data, receivecolor);
				}
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

// setupGraphics()
void setupGraphics(int* widthPtr, int* heightPtr) {	
	init(widthPtr,heightPtr);
	Start(*widthPtr, *heightPtr);		//Set up graphics, start picture
	Background(0,0,0);
}



EGLSurface createBufferSurface(int surfaceWidth, int surfaceHeight, VGImage* colorBuffer) {


}