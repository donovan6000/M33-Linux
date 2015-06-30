// Header files
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <termios.h>

using namespace std;


// Global variables
int fd = -1;
string firmwareVersion;
string serialNumber;
double backRightOffset;
double backLeftOffset;
double frontRightOffset;
double frontLeftOffset;
double bedHeightOffset;
double backlashX;
double backlashY;
double backRightOrientation;
double backLeftOrientation;
double frontRightOrientation;
double frontLeftOrientation;
uint8_t status;
string folderLocation;
uint8_t temperature = 215;


// Function prototypes

/*
Name: Initialize USB
Purpose: Initialized the USB device with a specified VID and PID
*/
bool initializeUSB(const char *vid, const char *pid);

/*
Name: Send Data
Purpose: Sends ASCII data to the device
*/
bool sendDataAscii(const char *data);

/*
Name: Send Data
Purpose: Sends binary data to the device
*/
bool sendDataBinary(const char *data);

/*
Name: Get Response
Purpose: Receives response from the device
*/
string getResponse();

/*
Name: Pre-process file
Purpose: Adjusts the input file to compensate for printer specific values
*/
bool preprocessFile();


// Main function
int main(int argc, char *argv[]) {

	// Initialize variables
	string response, line;
	fstream processedFile;
	ifstream input;
	char character;
	uint64_t totalLines = 0, lineCounter = 0;
	
	// Check if not root
    	if(getuid()) {

    		// Display error
		cout << "Elevated privileges required" << endl;
		return 0;
	}
	
	// Check if a file is provided
	if(argc >= 2) {
	
		// Check if file doesn't exists
		input.open(argv[1]);
		if(!input.good()) {
		
			// Display error
			cout << "Input file doesn't exist" << endl;
			return 0;
		}
	}
	
	// Wait for device to be connected
	while(!initializeUSB("03EB", "2404")) {
		cout << "M3D not detected" << endl;
		usleep(500000);
	}
	
	// Display message
	cout << "Connected to M3D" << endl;
	cout << "Initializing the device" << endl;
	
	// Put device into firmware mode
	while(1) {
		sendDataAscii("M115");
		while(read(fd, &character, 1) == -1);
		if(character != 'B')
			break;
		sendDataAscii("Q");
		initializeUSB("03EB", "2404");
	}
	
	// Get device info
	while(1) {
	
		// Send command until a valid response is received
		do {
			sendDataBinary("M115");
			response = getResponse();
		} while(response.substr(0, 2) != "ok");
	
		// Check if entire response was obtained
		if(response.find("SERIAL_NUMBER:") != string::npos && response.find("SERIAL_NUMBER:") == response.length() - 30) {
		
			// Set firmware and serial number 
			firmwareVersion = response.substr(response.find("FIRMWARE_VERSION:") + 17, response.find(" ", response.find("FIRMWARE_VERSION:")) - response.find("FIRMWARE_VERSION:") - 17);
			serialNumber = response.substr(response.find("SERIAL_NUMBER:") + 14);
			break;
		}
	}
	
	// Get bed offsets
	while(1) {
	
		// Send command until a valid response is received
		do {
			sendDataBinary("M578");
			response = getResponse();
		} while(response.substr(0, 2) != "ok");
		
		// Check if entire response was obtained
		if(response.find("ZO:") != string::npos && response.find("ZO:") <= response.length() - 9) {
		
			// Set bed offsets
			backRightOffset = stod(response.substr(response.find("BRO:") + 4, response.find(" ", response.find("BRO:")) - response.find("BRO:") - 4));
			backLeftOffset = stod(response.substr(response.find("BLO:") + 4, response.find(" ", response.find("BLO:")) - response.find("BLO:") - 4));
			frontRightOffset = stod(response.substr(response.find("FRO:") + 4, response.find(" ", response.find("FRO:")) - response.find("FRO:") - 4));
			frontLeftOffset = stod(response.substr(response.find("FLO:") + 4, response.find(" ", response.find("FLO:")) - response.find("FLO:") - 4));
			bedHeightOffset = stod(response.substr(response.find("ZO:") + 3));
			break;
		}
	}

	// Get backlash
	while(1) {
	
		// Send command until a valid response is received
		do {
			sendDataBinary("M572");
			response = getResponse();
		} while(response.substr(0, 2) != "ok");
		
		// Check if entire response was obtained
		if(response.find("BY:") != string::npos && response.find("BY:") <= response.length() - 9) {

			// Set backlash values
			backlashX = stod(response.substr(response.find("BX:") + 3, response.find(" ", response.find("BX:")) - response.find("BX:") - 3));
			backlashY = stod(response.substr(response.find("BY:") + 3));
			break;
		}
	}

	// Get bed orientation
	while(1) {
	
		// Send command until a valid response is received
		do {
			sendDataBinary("M573");
			response = getResponse();
		} while(response.substr(0, 2) != "ok");
		
		// Check if entire response was obtained
		if(response.find("FR:") != string::npos && response.find("FR:") <= response.length() - 9) {

			// Set bed orientation
			backRightOrientation = stod(response.substr(response.find("BR:") + 3, response.find(" ", response.find("BR:")) - response.find("BR:") - 3));
			backLeftOrientation = stod(response.substr(response.find("BL:") + 3, response.find(" ", response.find("BL:")) - response.find("BL:") - 3));
			frontLeftOrientation = stod(response.substr(response.find("FL:") + 3, response.find(" ", response.find("FL:")) - response.find("FL:") - 3));
			frontRightOrientation = stod(response.substr(response.find("FR:") + 3));
			break;
		}
	}
	
	// Get status
	while(1) {
	
		// Send command until a valid response is received
		do {
			sendDataBinary("M117");
			response = getResponse();
		} while(response.substr(0, 2) != "ok");
		
		// Check if entire response was obtained
		if(response.find("S:") != string::npos && response.find("S:") <= response.length() - 3) {

			// Set status
			status = stod(response.substr(response.find("S:") + 2));
			break;
		}
	}
	
	// Display device information
	cout << endl << "Firmware: " << firmwareVersion << endl;
	cout << "Serial Number: " << serialNumber << ' ';
	if(serialNumber.substr(0, 2) == "BK")
		cout << "Black";
	else if(serialNumber[0] == 'S')
		cout << "Silver";
	else if(serialNumber[0] == 'B')
		cout << "Blue";
	else if(serialNumber[0] == 'G')
		cout << "Green";
	else if(serialNumber[0] == 'O')
		cout << "Orange";
	cout << endl << "Back Right Offset: " << fixed << setprecision(4) << backRightOffset << endl;
	cout << "Back Left Offset: " << backLeftOffset << endl;
	cout << "Front Right Offset: " << frontRightOffset << endl;
	cout << "Front Left Offset: " << frontLeftOffset << endl;
	cout << "Bed Height Offset: " << bedHeightOffset << endl;
	cout << "Backlash X: " << backlashX << endl;
	cout << "Backlash Y: " << backlashY << endl;
	cout << "Back Right Orientation: " << backRightOrientation << endl;
	cout << "Back Left Orientation: " << backLeftOrientation << endl;
	cout << "Front Right Orientation: " << frontRightOrientation << endl;
	cout << "Front left Orientation: " << frontLeftOrientation << endl;
	cout << "Status: 0x" << hex << uppercase << static_cast<unsigned int>(status) << endl;
	cout << "Z Calibration Valid: " << (status & 0x02 ? "Yes" : "No") << endl;
	cout << endl << "Device is ready" << endl;
	
	// Check if a file was provided
	if(argc >= 2) {
	
		// Create temporary folder
		folderLocation = mkdtemp(const_cast<char *>(static_cast<string>("/tmp/M3D-XXXXXX").c_str()));
		
		// Check if creating processed file was successful
		processedFile.open(folderLocation + "/output.gcode", ios::out | ios::binary | ios::app);
		if(processedFile.good()) {
		
			// Display message
			cout << "Processing " << argv[1] << endl;
			
			// Read in input file
			processedFile << input.rdbuf();
			processedFile.close();
			
			// Check if preprocessing file was successful
			if(preprocessFile()) {
			
				// Determine the number of lines in the processed file
				processedFile.open(folderLocation + "/output.gcode", ios::in | ios::binary);
				while(processedFile.peek() != EOF) {
					getline(processedFile, line);
					totalLines++;
				}
				processedFile.clear();
				processedFile.seekg(0, ios::beg);
			
				// Display message
				cout << "Starting print" << endl;
			
				// Go through file
				while(processedFile.peek() != EOF) {

					// Get line
					getline(processedFile, line);
	
					// Display percent complete
					lineCounter++;
					cout << dec << lineCounter << '/' << totalLines << ' ' << static_cast<float>(lineCounter) / totalLines << '%' << endl;

					// Send line to the device
					do {
					
						// Display command
						cout << "Send: " << line.c_str() << endl;
						
						// Get next command if line didn't contain valid g-code
						if(!sendDataBinary(line.c_str()))
							break;
						
						// Get valid response
						response = getResponse();
						if(line.substr(0, 2) == "G0" && response == "Info:Too small")
							response = getResponse();
						
						// Display response
						cout << "Receive: " << response << endl << endl;
					} while(response.substr(0, 2) != "ok");
				}
			
				// Close processed file
				processedFile.close();
			}

			// Delete processed file
			unlink((folderLocation + "/output.gcode").c_str());
			
			// Display message
			cout << "Print finished" << endl;
		}

		// Delete temporary folder
		rmdir(folderLocation.c_str());
	}
	
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
			
				// Display command
				cout << "Send: " << line.c_str() << endl;
				
				// Get next command if line didn't contain valid g-code
				if(!sendDataBinary(line.c_str()))
					break;
				
				// Get valid response
				response = getResponse();
				if(line.substr(0, 2) == "G0" && response == "Info:Too small")
					response = getResponse();
				
				// Display response
				cout << "Receive: " << response << endl << endl;
			} while(response.substr(0, 2) != "ok");
		}
	}
	
	// Close file descriptor
	close(fd);
	
	// Return 0
	return 0;
}


// Supporting function implementation
bool initializeUSB(const char *vid, const char *pid) {

	// Initialize variables
	DIR *path;
        dirent *entry;
        string info;
        ifstream device;
        termios settings;
        
        // Check if path exists
        if((path = opendir("/sys/class/tty/"))) {
        
        	// Go through all tty devices
                while((entry = readdir(path)))
                
                	// Check if current device is a serial device
                        if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && !strncmp("ttyACM", entry->d_name, 6)) {
                        
                        	// Check if uevent file exists for the device
                        	device.open(static_cast<string>("/sys/class/tty/") + entry->d_name + "/device/uevent");
                        	if(device.good()) {
                        	
		                	// Read in file
					while(device.peek() != EOF)
						info.push_back(device.get());
					device.close();
					
					// Check if device has the specified pid and vid
					if(info.find(static_cast<string>("MODALIAS=usb:v") + vid + 'p' + pid) != string::npos) {

						// Close the file descriptor if it's already open
						if(fd != -1)
			       				close(fd);
			       			
			       			// Check if opening file was successful
						if((fd = open((static_cast<string>("/dev/") + entry->d_name).c_str(), O_RDWR | O_NONBLOCK)) != -1) {
						
							// Close path
			       				closedir(path);
			       				
			       				// Set serial protocol to 8n1 with 921600 baud rate
							memset(&settings, 0, sizeof(settings));
							settings.c_iflag = 0;
							settings.c_oflag = 0;
							settings.c_cflag= CS8 | CREAD | CLOCAL;
							settings.c_lflag = 0;
							settings.c_cc[VMIN] = 1;
							settings.c_cc[VTIME] = 5;
							cfsetospeed(&settings, B921600);
							cfsetispeed(&settings, B921600);

							// Apply settings after remaining data has been sent
							tcsetattr(fd, TCSAFLUSH, &settings);
							tcdrain(fd);	
			       				
			       				// Return true
			       				return true;
			       			}
					}
				}
			}
                
                // Close path
                closedir(path);
        }
	
	// Return false
	return false;
}

bool sendDataAscii(const char *data) {

	// Initialize variables
	bool returnValue;

	// Send data to the device
	tcflush(fd, TCIOFLUSH);
	returnValue = write(fd, data, strlen(data)) != -1;
	tcdrain(fd);
	
	// Return value
	return returnValue;
}

bool sendDataBinary(const char *data) {

	// Initialize variables
	bool returnValue;
	char parameterIdentifier = 0;
	vector<string> parameterValue(16);
	vector<uint8_t> request(4);
	string currentValue;
	char *commandStart;
	uint32_t dataType = 0x1080;
	uint16_t sum1 = 0, sum2 = 0;
	int32_t tempNumber, *tempPointer;
	float tempFloat;
	
	// Skip leading whitespace
	for(commandStart = const_cast<char *>(data); strlen(commandStart) && (*commandStart == ' ' || *commandStart == '\t'); commandStart++);
	
	// Check if data is a host command
	if(commandStart[0] == '@') {
	
		// Send unmodified data to the device
		tcflush(fd, TCIOFLUSH);
		returnValue = write(fd, commandStart, strlen(commandStart)) != -1;
	}
	
	// Otherwise
	else {
	
		// Go through data
		for(uint8_t i = 0; i <= strlen(commandStart); i++) {
		
			// Check if a parameter is detected
			if(i == 0 || commandStart[i - 1] == ' ' || !commandStart[i]) {
			
				// Check if value has been obtained for the parameter
				if(i) {
				
					// Enforce parameter order as N, M, G, X, Y, Z, E, F, T, S, P, I, J, R, D then string
					switch(parameterIdentifier) {
					
						case 'N':
						
							// Set data type
							dataType |= 1;
						
							// Store parameter value
							parameterValue[0] = currentValue;
						break;
						
						case 'M':
						
							// Set data type
							dataType |= (1 << 1);
						
							// Store parameter value
							parameterValue[1] = currentValue;
						break;
						
						case 'G':
						
							// Set data type
							dataType |= (1 << 2);
						
							// Store parameter value
							parameterValue[2] = currentValue;
						break;
						
						case 'X':
						
							// Set data type
							dataType |= (1 << 3);
						
							// Store parameter value
							parameterValue[3] = currentValue;
						break;
						
						case 'Y':
						
							// Set data type
							dataType |= (1 << 4);
						
							// Store parameter value
							parameterValue[4] = currentValue;
						break;
						
						case 'Z':
						
							// Set data type
							dataType |= (1 << 5);
							
							// Store parameter value
							parameterValue[5] = currentValue;
						break;
						
						
						case 'E':
						
							// Set data type
							dataType |= (1 << 6);
						
							// Store parameter value
							parameterValue[6] = currentValue;
						break;
						
						case 'F':
						
							// Set data type
							dataType |= (1 << 8);
						
							// Store parameter value
							parameterValue[7] = currentValue;
						break;
						
						case 'T':
						
							// Set data type
							dataType |= (1 << 9);
						
							// Store parameter value
							parameterValue[8] = currentValue;
						break;
						
						case 'S':
						
							// Set data type
							dataType |= (1 << 10);
						
							// Store parameter value
							parameterValue[9] = currentValue;
						break;
						
						case 'P':
						
							// Set data type
							dataType |= (1 << 11);
						
							// Store parameter value
							parameterValue[10] = currentValue;
						break;
						
						case 'I':
						
							// Set data type
							dataType |= (1 << 16);
						
							// Store parameter value
							parameterValue[11] = currentValue;
						break;
						
						case 'J':
						
							// Set data type
							dataType |= (1 << 17);
						
							// Store parameter value
							parameterValue[12] = currentValue;
						break;
						
						case 'R':
						
							// Set data type
							dataType |= (1 << 18);
						
							// Store parameter value
							parameterValue[13] = currentValue;
						break;
						
						case 'D':
						
							// Set data type
							dataType |= (1 << 19);
						
							// Store parameter value
							parameterValue[14] = currentValue;
						break;
					}
				}
				
				// Reset current value
				currentValue.clear();
				
				// Check if a string is required
				if(parameterIdentifier == 'M' && (parameterValue[1] == "23" || parameterValue[1] == "28" || parameterValue[1] == "29" || parameterValue[1] == "30" || parameterValue[1] == "32" || parameterValue[1] == "117")) {
				
					// Get string data
					for(; i < strlen(commandStart); i++)
						currentValue.push_back(commandStart[i]);
					
					// Check if a string exists
					if(currentValue.size()) {
					
						// Set data type
						dataType |= (1 << 15);
					
						// Store parameter value
						parameterValue[15] = currentValue;
					
						// Set fifth byte of request to string length
						request.push_back(currentValue.size());
					}
				}
				
				// Check if a comment is detected
				if(commandStart[i] == ';')
			
					// Stop parsing line
					break;
				
				// Set parameter identifier
				parameterIdentifier = commandStart[i];
			}
		
			// Otherwise
			else if(commandStart[i] != ' ' && commandStart[i] != '\t')
		
				// Get current value
				currentValue.push_back(commandStart[i]);
		}
		
		// Check if command was empty
		if(dataType == 0x1080)
		
			// Return false
			return false;
		
		// Fill first four bytes of request to data type
		request[0] = dataType;
		request[1] = dataType >> 8;
		request[2] = dataType >> 16;
		request[3] = dataType >> 24;
	
		// Go through all parameters
		for(uint8_t i = 0; i < parameterValue.size(); i++)
		
			// Check if current parameter is set
			if(parameterValue[i].size()) {
			
				// Determine parameter command
				switch(i) {
			
					// Check if N, M, or G command
					case 0:
					case 1:
					case 2:
				
						// Set 2 byte integer parameter value
						tempNumber = atoi(parameterValue[i].c_str());
						request.push_back(tempNumber & 0xFF);
						request.push_back((tempNumber >> 8) & 0xFF);
					break;
					
					// Check if X, Y, Z, E, F, I, J, R or D command
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 11:
					case 12:
					case 13:
					case 14:
					
						// Set 4 byte float parameter value
						tempFloat = atof(parameterValue[i].c_str());
						tempPointer = (int32_t*)&tempFloat;
						request.push_back(*tempPointer & 0xFF);
						request.push_back((*tempPointer >> 8) & 0xFF);
						request.push_back((*tempPointer >> 16) & 0xFF);
						request.push_back((*tempPointer >> 24) & 0xFF);
					break;
					
					// Check if T command
					case 8:
					
						// Set 1 byte integer parameter value
						tempNumber = atoi(parameterValue[i].c_str());
						request.push_back(tempNumber & 0xFF);
					break;
					
					// Check if S or P command
					case 9:
					case 10:
					
						// Set 4 byte integer parameter value
						tempNumber = atoi(parameterValue[i].c_str());
						request.push_back(tempNumber & 0xFF);
						request.push_back((tempNumber >> 8) & 0xFF);
						request.push_back((tempNumber >> 16) & 0xFF);
						request.push_back((tempNumber >> 24) & 0xFF);
					break;
					
					// Check if string command
					case 15:
					
						// Set string parameter value
						for(uint8_t j = 0; j < parameterValue[i].length(); j++)
							request.push_back(parameterValue[i][j]);
					break;
				}
			}
	
		// Go through all values
		for(uint8_t index = 0; index < request.size(); index++) {

			// Set sums
			sum1 = (sum1 + request[index]) % 0xFF;
			sum2 = (sum1 + sum2)  % 0xFF;
		}

		// Append Fletcher 16 checksum checksum to request
		request.push_back(sum1);
		request.push_back(sum2);
			
		// Send request to the device
		tcflush(fd, TCIOFLUSH);
		returnValue = write(fd, request.data(), request.size()) != -1;
	}
	
	// Wait until request is sent
	tcdrain(fd);
	
	// Return value
	return returnValue;
}

string getResponse() {

	// Initialize variables
	string response;
	char character;
	
	// Get response
	do {
		while(read(fd, &character, 1) == -1);
		response.push_back(character);
	} while(character != '\n');
	
	// Remove newline character from response
	response.pop_back();
	
	// Return response
	return response;
}

bool preprocessFile() {

	// Initialzie variables
	string line;
	fstream processedFile(folderLocation + "/output.gcode", ios::in | ios::binary);
	fstream temp(folderLocation + "/temp", ios::out | ios::in | ios::binary | ios::app);
	
	// Check if temp file was opened successfully
	if(processedFile.good() && temp.good()) {
	
		// Add intro to temp
		temp << "M106 S255" << endl;
		temp << "M17" << endl;
		temp << "G90" << endl;
		temp << "M104 S" << to_string(temperature) << endl;
		temp << "G0 Z5 F2900" << endl;
		temp << "G28" << endl;
		temp << "M18" << endl;
		temp << "M109 S" << to_string(temperature) << endl;
		temp << "G4 S10" << endl;
		temp << "M17" << endl;
		temp << "G91" << endl;
		temp << "G0 E7.5 F2000" << endl;
		temp << "G92 E0" << endl;
		temp << "G90" << endl;
		temp << "G0 F2400" << endl;
	
		// Go through processed file
		while(processedFile.peek() != EOF) {
	
			// Read in line
			getline(processedFile, line);
			line.pop_back();
			
			// Remove leading white space
			while(line[0] == ' ' || line[0] == '\t')
				line.erase(0, 1);
			
			// Check if line begins layer 0
			if(line == ";LAYER:0")
			
				// Increase temperature
				temp << "M109 S" << to_string(temperature + 10) << endl;
				
			// Otherwise check if line begins layer 1
			else if(line == ";LAYER:1")
			
				// Descrease temperature
				temp << "M109 S" << to_string(temperature) << endl;
			
			// Remove trailing comments
			if(line[0] != ';' && line.find(";") != string::npos)
				line.erase(line.find(";"));
		
			// Remove fan controls
			if(line.length() >= 4 && (line.substr(0, 4) == "M106" || line.substr(0, 4) == "M107"))
				continue;
			
			// Remove T commands
			while(line[0] != ';' && line.find("T") != string::npos)
				line.erase(line.find("T"), line.find(" ", line.find("T")) - line.find("T") + 1);
			
			// Remove not applicable commands
			if(line.length() >= 3 && line.substr(0, 3) == "M82")
				continue;
			
			// Remove trailing white space
			while(line.back() == ' ' || line.back() == '\t')
				line.pop_back();
			
			// Remove empty lines
			if(!line.length())
				continue;
			
			// Send line to temp
			temp << line << endl;
		}
	
		// Add outro to temp
		temp << "G91" << endl;
		temp << "G0 E-1 F2000" << endl;
		temp << "G0 X5 Y5 F2000" << endl;
		temp << "G0 E-8 F2000" << endl;
		temp << "M104 S0" << endl;
		temp << "G0 Z3 F2900" << endl;
		temp << "G90" << endl;
		temp << "G0 X95 Y95" << endl;
		temp << "M18" << endl;
		
		// Transfer contents of temp to processed file
		temp.clear();
		temp.seekg(0, ios::beg);
		processedFile.close();
		processedFile.open(folderLocation + "/output.gcode", ios::out | ios::binary | ios::trunc);
		processedFile << temp.rdbuf();
		processedFile.close();
		
		// Delete temp
		unlink((folderLocation + "/temp").c_str());
		
		// Return true
		return true;
	}
	
	// Return false
	return false;
}
