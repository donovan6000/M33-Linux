// Header files
#include <iostream>
#include <fstream>
#include <cstring>
#include "printer.h"

using namespace std;


// Main function
int main(int argc, char *argv[]) {

	// Initialize variables
	Printer printer;
	string response, line, firmwareRom, gcodeFile;
	ifstream file;
	bool translate = false;
	
	// Display version
	cout << "M3D Linux V0.8" << endl << endl;
	
	// Go through all commands
	for(uint8_t i = 0; i < argc; i++) {
	
		// Check if help is requested
		if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
		
			// Display help
			cout << "Usage: m3d-linux -v -p -w -t -b -l -f -r firmware.rom -g file.gcode -s" << endl;
			cout << "-v | --validation: Use validation pre-processor" << endl;
			cout << "-p | --preparation: Use preparation pre-processor" << endl;
			cout << "-w | --wavebonding: Use wave bonding pre-processor" << endl;
			cout << "-t | --thermalbonding: Use thermal bonding pre-processor" << endl;
			cout << "-b | --bedcompensation: Use bed compensation pre-processor" << endl;
			cout << "-l | --backlashcompensation: Use backlash compensation pre-processor" << endl;
			cout << "-f | --feedrateconversion: Use feed rate conversion pre-processor" << endl;
			cout << "-r | --firmwarerom: Use the following parameter as the firmware ROM in case firmware is corrupt" << endl;
			cout << "-g | --gcodefile: Use the following parameter as the G-code file to processes and send to the printer. G-code commands can be manually entered if not G-code file is not provided." << endl;
			cout << "-s | --translate: Uses program as a middle man to communicate between the printer and other software" << endl << endl;
			return 0;
		}
	
		// Otherwise check if using validation pre-processor
		else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--validation"))
		
			// Set validation preprocessor
			printer.setValidationPreprocessor();
		
		// Otherwise check if using preparation pre-processor
		else if(!strcmp(argv[i], "-p") || !strcmp(argv[i], "--preparation"))
		
			// Set preparation preprocessor
			printer.setPreparationPreprocessor();
		
		// Otherwise check if using wave bonding pre-processor
		else if(!strcmp(argv[i], "-w") || !strcmp(argv[i], "--wavebonding"))
		
			// Set wave bonding preprocessor
			printer.setWaveBondingPreprocessor();
		
		// Otherwise check if using thermal bonding pre-processor
		else if(!strcmp(argv[i], "-t") || !strcmp(argv[i], "--thermalbonding"))
		
			// Set thermal bonding preprocessor
			printer.setThermalBondingPreprocessor();
		
		// Otherwise check if using bed compensation pre-processor
		else if(!strcmp(argv[i], "-b") || !strcmp(argv[i], "--bedcompensation"))
		
			// Set bed compensation preprocessor
			printer.setBedCompensationPreprocessor();
		
		// Otherwise check if using backlash compensation pre-processor
		else if(!strcmp(argv[i], "-l") || !strcmp(argv[i], "--backlashcompensation"))
		
			// Set backlash compensation preprocessor
			printer.setBacklashCompensationPreprocessor();
		
		// Otherwise check if using feed rate conversion pre-processor
		else if(!strcmp(argv[i], "-f") || !strcmp(argv[i], "--feedrateconversion"))
		
			// Set feed rate conversion preprocessor
			printer.setFeedRateConversionPreprocessor();
		
		// Otherwise check if a firmware rom is provided
		else if(!strcmp(argv[i], "-r") || !strcmp(argv[i], "--firmwarerom")) {
		
			// Check if firmware rom parameter exists
			if(i < argc - 1)
			
				// Set firmware rom
				firmwareRom = argv[++i];
			
			// Otherwise
			else {
			
				// Display error
				cout << "No firmware ROM provided" << endl;
				return 0;
			}
		}
		
		// Otherwise check if a g-code file is provided
		else if(!strcmp(argv[i], "-g") || !strcmp(argv[i], "--gcodefile")) {
		
			// Check if g-code file parameter exists
			if(i < argc - 1)
			
				// Set g-code file
				gcodeFile = argv[++i];
			
			// Otherwise
			else {
			
				// Display error
				cout << "No G-code file provided" << endl;
				return 0;
			}
		}
		
		// Otherwise check if using with octoprint
		else if(!strcmp(argv[i], "-s") || !strcmp(argv[i], "--translate"))
		
			// Set translate
			translate = true;
	}
	
	// Check if a firmware rom is provided
	if(!firmwareRom.empty()) {
	
		// Check if firmware rom doesn't exists
		file.open(firmwareRom);
		if(!file.good()) {
		
			// Display error
			cout << "Firmware ROM doesn't exist" << endl;
			return 0;
		}
		
		// Close file
		file.close();
	}
	
	// Check if a g-code file is provided
	if(!gcodeFile.empty()) {
	
		// Check if g-code file doesn't exists
		file.open(gcodeFile);
		if(!file.good()) {
		
			// Display error
			cout << "G-code file doesn't exist" << endl;
			return 0;
		}
		
		// Close file
		file.close();
	}
	
	// Wait for device to be connected
	cout << "Attempting to connect to the printer" << endl;
	while(!printer.connect())
		cout << "Printer not detected" << endl;
	
	// Display message
	cout << "Connected to printer" << endl;
	cout << "Initializing the device" << endl;
	
	// Check if printer's firmware isn't valid
	if(!printer.isFirmwareValid()) {
	
		// Display error
		cout << "Printer's firmware is corrupt" << endl;
		
		// Check if a firmware rom is provided
		if(!firmwareRom.empty()) {
		
			// Display message
			cout << "Updating firmware" << endl;
		
			// Check if updating printer's firmware failed
			if(!printer.updateFirmware(firmwareRom.c_str())) {
		
				// Display error
				cout << "Failed to update firmware" << endl;
				return 0;
			}
		}
	}
	
	// Check if collect printer information failed
	if(!printer.collectInformation()) {
	
		// Display error
		cout << "Failed to collect printer information" << endl;
		return 0;
	}
	
	// Check if printer Z isn't valid
	if(!printer.isZValid()) {
	
		// Display error
		cout << "Printer's Z is invalid" << endl << "Calibrating Z" << endl;
	
		// Calibrate Z
		printer.calibrateZ();
	}
	
	// Check if printer bed orientation isn't valid
	if(!printer.isBedOrientationValid()) {
	
		// Display error
		cout << "Printer's bed orientation is invalid" << endl << "Calibrating bed orientation" << endl;
	
		// Calibrate bed orientation
		printer.calibrateBedOrientation();
	}
	
	// Check if a file was provided
	if(!gcodeFile.empty())
	
		// Print file
		printer.printFile(gcodeFile.c_str());
	
	// Otherwise check if translating
	else if(translate)
	
		// Enter translator mode
		printer.translatorMode();
	
	// Otherwise
	else {
	
		// Display message
		cout << "Entering manual G-code mode" << endl << "Enter 'quit' to exit" << endl;
		
		// Loop forever
		while(1) {
	
			// Get g-code from user
			cout << "Enter command: ";
			getline(cin, line);
			
			// End if requested
			if(line == "quit")
				break;
		
			// Send g-code to device
			do {
				
				// Get next command if line didn't contain valid g-code
				if(!printer.sendRequest(line)) {
				
					cout << "Failed to parse command" << endl << endl;
					break;
				}
				
				// Display command
				cout << "Send: " << line << endl;
				
				// Get valid response
				do {
					response = printer.receiveResponse();
					while(response == "Info:Too small" || response.substr(0, 2) == "T:")
						response = printer.receiveResponse();
				} while(response.empty());
				
				// Display response
				cout << "Receive: " << response << endl << endl;
			} while(response.substr(0, 2) != "ok");
		}
	}
	
	// Return 0
	return 0;
}
