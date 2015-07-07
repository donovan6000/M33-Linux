// Header files
#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <cfloat>
#include <unistd.h>
#include "printer.h"
#include "gcode.h"

using namespace std;


// Global variables
Printer printer;
string folderLocation;

uint8_t temperature = 215;
filamentTypes filament = PLA;

double minXModel = DBL_MAX, minYModel = DBL_MAX, minZModel = DBL_MAX;
double maxXModel = 0, maxYModel = 0, maxZModel = 0;
double minXExtruder = DBL_MAX, minYExtruder = DBL_MAX, minZExtruder = DBL_MAX;
double maxXExtruder = 0, maxYExtruder = 0, maxZExtruder = 0;
double minFeedRate = 0, maxFeedRate = DBL_MAX;

bool useBasicPreparation = true;
bool useWaveBonding = false;
bool useThermalBonding = true;
bool useBedCompensation = true;
bool useBacklashCompensation = true;
bool useFeedRateConversion = true;

// Wave bonding
#define WAVE_PERIOD 5
#define WAVE_PERIOD_QUARTER WAVE_PERIOD / 4L
#define WAVE_SIZE 0.15L

// Thermal and wave bonding
#define BONDING_HEIGHT_OFFSET -0.1L

// Bed compensation
#define CHANGE_IN_HEIGHT_THAT_DOUBLES_EXTRUSION 0.15L
#define CHANGE_EXTRUSION_TO_COMPENSATE false
#define FIRST_LAYER_ONLY false
#define LEVELLING_MOVE_X 104.9L
#define LEVELLING_MOVE_Y 103L
#define MOVE_Z_TO_COMPENSATE true
#define PROBE_Z_DISTANCE 55L
#define SEGMENT_LENGTH 2L

// Feed rate conversion
#define MAX_FEED_RATE 60.0001L

enum direction {POSITIVE, NEGATIVE, NEITHER};
enum printTiers {LOW, MEDIUM, HIGH};


// Function prototypes

/*
Name: Get Print Dimensions
Purpose: Calculates X, Y, and Z dimensions of the model and where the extruder moves
*/
bool getPrintInformation();

/*
Name: Basic Preparation Preprocessor
Purpose: Adjusts the input file to remove unwanted commands and adds printer specific starting and ending command sequences
*/
bool basicPreparationPreprocessor();

/*
Name: Wave Bonding Preprocessor
Purpose: Adjusts the input file to incoporate wave bonding
*/
bool waveBondingPreprocessor();

/*
Name: Thermal Bonding Preprocessor
Purpose: Adjusts the input file to incoporate thermal bonding
*/
bool thermalBondingPreprocessor();

/*
Name: Bed Compensation Preprocessor
Purpose: Adjusts the input file to incoporate bed compensation
*/
bool bedCompensationPreprocessor();

/*
Name: Backlash Compensation Preprocessor
Purpose: Adjusts the input file to incoporate backlash compensation
*/
bool backlashCompensationPreprocessor();

/*
Name: Feed Rate Conversion Preprocessor
Purpose: Adjusts the input file to incoporate feed rate conversion
*/
bool feedRateConversionPreprocessor();


// Main function
int main(int argc, char *argv[]) {

	// Initialize variables
	string response, line;
	fstream processedFile;
	ifstream input;
	uint64_t totalLines = 0, lineCounter = 0;
	
	// Display version
	cout << "M3D Linux V0.5" << endl;
	
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
	while(!printer.connect())
		cout << "M3D not detected" << endl;
	
	// Display message
	cout << "Connected to M3D" << endl;
	cout << "Initializing the device" << endl;
	
	// Check if printer's firmware isn't valid
	if(!printer.isFirmwareValid()) {
	
		// Display error
		cout << "Printer firmware is corrupt" << endl << "Updating firmware" << endl;
		
		// Check if updating printer's firmware failed
		if(!printer.updateFirmware("2015062401.rom")) {
		
			// Display error
			cout << "Failed to update firmware" << endl;
			return 0;
		}
	}
	
	// Check if collect printer information failed
	if(!printer.collectInformation()) {
	
		// Display error
		cout << "Failed to collect printer information" << endl;
		return 0;
	}
	
	// Check if a file was provided
	if(argc >= 2) {
	
		// Create temporary folder
		folderLocation = mkdtemp(const_cast<char *>(static_cast<string>("/tmp/m3d-XXXXXX").c_str()));
		
		// Check if creating processed file was successful
		processedFile.open(folderLocation + "/output.gcode", ios::out | ios::binary | ios::app);
		if(processedFile.good()) {
		
			// Display message
			cout << "Processing " << argv[1] << endl;
			
			// Read in input file
			processedFile << input.rdbuf();
			processedFile.close();

			// Get print dimensions
			getPrintInformation();
			
			// Use preparation preprocessor if set
			if(useBasicPreparation)
				basicPreparationPreprocessor();
			
			// Use wave bonding preprocessor if set
			if(useWaveBonding)
				waveBondingPreprocessor();
			
			// Use thermal bonding preprocessor if set
			if(useThermalBonding)
				thermalBondingPreprocessor();
			
			// Use bed compensation preprocessor if set
			if(useBedCompensation)
				bedCompensationPreprocessor();
			
			// Use backlash compensation preprocessor if set
			if(useBacklashCompensation)
				backlashCompensationPreprocessor();
			
			// Use feed rate conversion proprocessor if set
			if(useFeedRateConversion)
				feedRateConversionPreprocessor();
			
			// Determine the number of command lines in the fully processed file
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
					
					// Get next command if line didn't contain valid g-code
					if(!printer.sendRequest(line)) {
					
						cout << "Failed to parse " << line << endl << endl;
						break;
					}
					
					// Display command
					cout << "Send: " << line << endl;
					
					// Get valid response
					do {
						response = printer.receiveResponse();
						if(line.substr(0, 2) == "G0" && response == "Info:Too small")
							response = printer.receiveResponse();
					} while(response.empty());
					
					// Display response
					cout << "Receive: " << response << endl << endl;
				} while(response.substr(0, 2) != "ok");
			}
			
			// Close processed file
			processedFile.close();
			
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
					if(line.substr(0, 2) == "G0" && response == "Info:Too small")
						response = printer.receiveResponse();
				} while(response.empty());
				
				// Display response
				cout << "Receive: " << response << endl << endl;
			} while(response.substr(0, 2) != "ok" && response.substr(0, 2) != "T:");
		}
	}
	
	// Return 0
	return 0;
}


// Supporting function implementation
bool getPrintInformation() {

	// Initialize variables
	string line;
	Gcode gcode;
	fstream file(folderLocation + "/output.gcode", ios::in | ios::binary);
	double localX = 54, localY = 60, localZ = 0, localE = 0, commandX, commandY, commandZ, commandE, commandF;
	bool relativeMode = true, positiveExtrusion;
	printTiers tier = LOW;
	
	// Check if file was opened successfully
	if(file.good()) {
	
		// Go through file
		while(file.peek() != EOF) {
	
			// Read in line
			getline(file, line);
			
			// Check if line was parsed successfully and it's a G command
			if(gcode.parseLine(line) && gcode.hasValue('G')) {
			
				// Check what parameter is associated with the command
				switch(stoi(gcode.getValue('G'))) {
				
					case 0:
					case 1:
					
						// Clear positive extruding
						positiveExtrusion = false;
						
						// Check if command has an E value
						if(gcode.hasValue('E')) {
						
							// Get E value of the command
							commandE = stod(gcode.getValue('E'));
							
							// Set positive extrusion based on adjusted E value
							if(relativeMode) {
								positiveExtrusion = commandE > 0;
								localE += commandE;
							}
							
							else {
								positiveExtrusion = commandE > localE;
								localE = commandE;
							}
						}
						
						// Check if command has a F value
						if(gcode.hasValue('F')) {
						
							// Get F value of the command
							commandF = stod(gcode.getValue('F'));
						
							// Update minimum and maximum feed rate values
							minFeedRate = minFeedRate < commandF ? minFeedRate : commandF;
							maxFeedRate = maxFeedRate > commandF ? maxFeedRate : commandF;
						}
						
						// Check if positive extruding
						if(positiveExtrusion) {
						
							// Update minimums and maximums dimensions of the model
							minXModel = minXModel < localX ? minXModel : localX;
							maxXModel = maxXModel > localX ? maxXModel : localX;
							minYModel = minYModel < localY ? minYModel : localY;
							maxYModel = maxYModel > localY ? maxYModel : localY;
							minZModel = minZModel < localZ ? minZModel : localZ;
							maxZModel = maxZModel > localZ ? maxZModel : localZ;
						}
						
						// Update minimums and maximums dimensions of extruder
						minXExtruder = minXExtruder < localX ? minXExtruder : localX;
						maxXExtruder = maxXExtruder > localX ? maxXExtruder : localX;
						minYExtruder = minYExtruder < localY ? minYExtruder : localY;
						maxYExtruder = maxYExtruder > localY ? maxYExtruder : localY;
						minZExtruder = minZExtruder < localZ ? minZExtruder : localZ;
						maxZExtruder = maxZExtruder > localZ ? maxZExtruder : localZ;
						
						// Check if command has an X value
						if(gcode.hasValue('X')) {
						
							// Get X value of the command
							commandX = stod(gcode.getValue('X'));
							
							// Set local X
							localX = relativeMode ? localX + commandX : commandX;
						}
						
						// Check if command has a Y value
						if(gcode.hasValue('Y')) {
						
							// Get Y value of the command
							commandY = stod(gcode.getValue('Y'));
							
							// Set local Y
							localY = relativeMode ? localY + commandY : commandY;
						}
						
						// Check if command has a X value
						if(gcode.hasValue('Z')) {
						
							// Get X value of the command
							commandZ = stod(gcode.getValue('Z'));
							
							// Set local Z
							localZ = relativeMode ? localZ + commandZ : commandZ;
							
							// Check if Z is out of bounds
							if(localZ < BED_LOW_MIN_Z || localZ > BED_HIGH_MAX_Z)
							
								// Return false
								return false;
							
							// Set print tier
							if(localZ >= BED_LOW_MIN_Z && localZ < BED_LOW_MAX_Z)
								tier = LOW;
								
							else if(localZ >= BED_LOW_MIN_Z && localZ < BED_MEDIUM_MAX_Z)
								tier = MEDIUM;
								
							else if(localZ >= BED_HIGH_MIN_Z && localZ <= BED_HIGH_MAX_Z)
								tier = HIGH;
							
						}
						
						// Return false if X or Y are out of bounds				
						switch(tier) {
							case LOW:
							
								if(localX < BED_LOW_MIN_X || localX > BED_LOW_MAX_X || localY < BED_LOW_MIN_Y || localY > BED_LOW_MAX_Y)
									return false;
							break;
							
							case MEDIUM:
							
								if(localX < BED_MEDIUM_MIN_X || localX > BED_MEDIUM_MAX_X || localY < BED_MEDIUM_MIN_Y || localY > BED_MEDIUM_MAX_Y)
									return false;
							break;

							case HIGH:
							
								if(localX < BED_HIGH_MIN_X || localX > BED_HIGH_MAX_X || localY < BED_HIGH_MIN_Y || localY > BED_HIGH_MAX_Y)
									return false;
							break;
						}
						
						// Check if positive extruding
						if(positiveExtrusion) {
						
							// Update minimums and maximums dimensions of the model
							minXModel = minXModel < localX ? minXModel : localX;
							maxXModel = maxXModel > localX ? maxXModel : localX;
							minYModel = minYModel < localY ? minYModel : localY;
							maxYModel = maxYModel > localY ? maxYModel : localY;
							minZModel = minZModel < localZ ? minZModel : localZ;
							maxZModel = maxZModel > localZ ? maxZModel : localZ;
						}
						
						// Update minimums and maximums dimensions of extruder
						minXExtruder = minXExtruder < localX ? minXExtruder : localX;
						maxXExtruder = maxXExtruder > localX ? maxXExtruder : localX;
						minYExtruder = minYExtruder < localY ? minYExtruder : localY;
						maxYExtruder = maxYExtruder > localY ? maxYExtruder : localY;
						minZExtruder = minZExtruder < localZ ? minZExtruder : localZ;
						maxZExtruder = maxZExtruder > localZ ? maxZExtruder : localZ;
					break;
					
					case 90:
					
						// Clear relative mode
						relativeMode = false;
					break;
				
					case 91:
					
						// Set relative mode
						relativeMode = true;
					break;
				}
			}
		}
		
		// Return true
		return true;
	}
	
	// Return false
	return false;
}

uint16_t getBoundedTemperature(uint16_t temperature) {

	// Return temperature bounded by range
	return temperature > 285 ? 285 : temperature < 150 ? 150 : temperature;
}

double getDistance(const Gcode &firstPoint, const Gcode &secondPoint) {

	// Initialize variables
	double firstX = firstPoint.hasValue('X') ? stod(firstPoint.getValue('X')) : 0;
	double firstY = firstPoint.hasValue('Y') ? stod(firstPoint.getValue('Y')) : 0;
	double secondX = secondPoint.hasValue('X') ? stod(secondPoint.getValue('X')) : 0;
	double secondY = secondPoint.hasValue('Y') ? stod(secondPoint.getValue('Y')) : 0;

	// Return distance between the two values
	return sqrt(pow(firstX - secondX, 2) + pow(firstY - secondY, 2));
}

Gcode createTackPoint(const Gcode &point, const Gcode &refrence) {

	// Initialize variables
	Gcode gcode;
	uint32_t time = ceil(getDistance(point, refrence));
	
	// Check if time is greater than 5
	if(time > 5) {
	
		// Set g-code to a delay command based on time
		gcode.setValue('G', "4");
		gcode.setValue('P', to_string(time));
	}
	
	// Return gcode
	return gcode;
}

bool isSharpCornerForThermalBonding(const Gcode &point, const Gcode &refrence) {

	// Initialize variables
	double currentX = point.hasValue('X') ? stod(point.getValue('X')) : 0;
	double currentY = point.hasValue('Y') ? stod(point.getValue('Y')) : 0;
	double previousX = refrence.hasValue('X') ? stod(refrence.getValue('X')) : 0;
	double previousY = refrence.hasValue('Y') ? stod(refrence.getValue('Y')) : 0;
	
	// Calculate value
	double value = acos((currentX * previousX + currentY * previousY) / (pow(currentX * currentX + currentY * currentY, 2) * pow(previousX * previousX + previousY * previousY, 2)));
	
	// Return if sharp corner
	return value > 0 && value < M_PI_2;
}

bool isSharpCornerForWaveBonding(const Gcode &point, const Gcode &refrence) {

	// Initialize variables
	double currentX = point.hasValue('X') ? stod(point.getValue('X')) : 0;
	double currentY = point.hasValue('Y') ? stod(point.getValue('Y')) : 0;
	double previousX = refrence.hasValue('X') ? stod(refrence.getValue('X')) : 0;
	double previousY = refrence.hasValue('Y') ? stod(refrence.getValue('Y')) : 0;
	
	// Calculate value
	double value = acos((currentX * previousX + currentY + previousY) / (pow(currentX * currentX + currentY + currentY, 2) * pow(previousX * previousX + previousY + previousY, 2)));
	
	// Return if sharp corner
	return value > 0 && value < M_PI_2;
}

double getCurrentAdjustmentZ() {

	// Initialize variables
	static uint8_t waveStep = 0;

	// Set adjustment
	double adjustment = waveStep ? waveStep != 2 ? 0 : -1.5 : 1;
	
	// Increment wave step
	waveStep = (waveStep + 1) % 4;
	
	// Return adjustment
	return adjustment * WAVE_SIZE;
}

double getHeightAdjustmentRequired(double x, double y) {

	// Initialize variables
	double left = (printer.getBackLeftOffset() - printer.getFrontLeftOffset()) / LEVELLING_MOVE_Y;
	double right = (printer.getBackRightOffset() - printer.getFrontRightOffset()) / LEVELLING_MOVE_Y;
	
	// Return height adjustment
	return (right * y + printer.getFrontRightOffset() - (left * y + printer.getFrontLeftOffset())) / LEVELLING_MOVE_X * x + (left * y + printer.getFrontLeftOffset());
}

bool basicPreparationPreprocessor() {

	// Initialzie variables
	string line;
	Gcode gcode;
	fstream processedFile(folderLocation + "/output.gcode", ios::in | ios::binary);
	fstream temp(folderLocation + "/temp", ios::out | ios::in | ios::binary | ios::app);
	
	// Check if temp file was opened successfully
	if(processedFile.good() && temp.good()) {
	
		// Add intro to temp
		temp << "M106 S" << (filament == PLA ? "255" : "50") << endl;
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
		temp << "; can extrude" << endl;
	
		// Go through processed file
		while(processedFile.peek() != EOF) {
	
			// Read in line
			getline(processedFile, line);
			
			// Pase line
			gcode.parseLine(line);
			
			// Check if command controls extruder temperature or fan speed
			if(gcode.hasValue('M') && (gcode.getValue('M') == "104" || gcode.getValue('M') == "106" || gcode.getValue('M') == "107" || gcode.getValue('M') == "109"))
			
				// Get next line
				continue;
			
			// Send line to temp
			temp << gcode << endl;
		}
	
		// Add outro to temp
		temp << "G91" << endl;
		temp << "G0 E-1 F2000" << endl;
		temp << "G0 X5 Y5 F2000" << endl;
		temp << "G0 E-8 F2000" << endl;
		temp << "M104 S0" << endl;
		if(maxZExtruder > 60) {
			if(maxZExtruder < 110)
				temp << "G0 Z3 F2900" << endl;
			temp << "G90" << endl;
			temp << "G0 X90 Y84" << endl;
		}
		else {
			temp << "G0 Z3 F2900" << endl;
			temp << "G90" << endl;
			temp << "G0 X95 Y95" << endl;
		}
		temp << "M18" << endl;
		temp << "M107" << endl;
		
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

bool waveBondingPreprocessor() {

	// Initialzie variables
	string line;
	Gcode gcode, previousGcode, refrenceGcode, tackPoint, extraGcode;
	fstream processedFile(folderLocation + "/output.gcode", ios::in | ios::binary);
	fstream temp(folderLocation + "/temp", ios::out | ios::in | ios::binary | ios::app);
	bool relativeMode = true;
	bool firstLayer = true, changesPlane = false;
	uint32_t cornerCounter = 0, layerNumber, baseLayer = 0, waveRatio;
	double distance;
	double positionAbsoluteX = 0, positionAbsoluteY = 0, positionAbsoluteZ = 0, positionAbsoluteE = 0;
	double positionRelativeX = 0, positionRelativeY = 0, positionRelativeZ = 0, positionRelativeE = 0;
	double deltaX, deltaY, deltaZ, deltaE;
	double tempRelativeX, tempRelativeY, tempRelativeZ, tempRelativeE;
	double relativeDifferenceX, relativeDifferenceY, relativeDifferenceZ, relativeDifferenceE;
	double deltaRatioX, deltaRatioY, deltaRatioZ, deltaRatioE;
	
	// Check if temp file was opened successfully
	if(processedFile.good() && temp.good()) {
	
		// Go through processed file
		while(processedFile.peek() != EOF) {
	
			// Read in line
			getline(processedFile, line);
			
			// Check if line is a layer command
			if(line.find(";LAYER:") != string::npos) {
			
				// Set layer number
				layerNumber = stoi(line.substr(7));
				
				// Set base number is layer number is less than it
				if(layerNumber < baseLayer)
					baseLayer = layerNumber;
				
				// Set first layer
				firstLayer = layerNumber == baseLayer;
			}
			
			// Check is line was parsed, it contains G0 or G1, and it's not in relative mode
			if(gcode.parseLine(line) && gcode.hasValue('G') && (gcode.getValue('G') == "0" || gcode.getValue('G') == "1") && !relativeMode) {
			
				// Check if line contains an X or Y value
				if(gcode.hasValue('X') || gcode.hasValue('Y'))
				
					// Set changes plane
					changesPlane = true;
				
				// Check if line contains a Z value and is in the first layer
				if(gcode.hasValue('Z') && firstLayer)
				
					// Adjust Z value by height offset
					gcode.setValue('Z', to_string(stod(gcode.getValue('Z')) + BONDING_HEIGHT_OFFSET));
				
				// Set delta values
				deltaX = !gcode.hasValue('X') ? 0 : stod(gcode.getValue('X')) - positionRelativeX;
				deltaY = !gcode.hasValue('Y')? 0 : stod(gcode.getValue('Y')) - positionRelativeY;
				deltaZ = !gcode.hasValue('Z') ? 0 : stod(gcode.getValue('Z')) - positionRelativeZ;
				deltaE = !gcode.hasValue('E') ? 0 : stod(gcode.getValue('E')) - positionRelativeE;
				
				// Adjust position absolute and relative values for the changes
				positionAbsoluteX += deltaX;
				positionAbsoluteY += deltaY;
				positionAbsoluteZ += deltaZ;
				positionAbsoluteE += deltaE;
				positionRelativeX += deltaX;
				positionRelativeY += deltaY;
				positionRelativeZ += deltaZ;
				positionRelativeE += deltaE;
				
				// Calculate distance of change
				distance = sqrt(deltaX * deltaX + deltaY * deltaY);
				
				// Set wave ratio
				waveRatio = distance > WAVE_PERIOD_QUARTER ? distance / WAVE_PERIOD_QUARTER : 1;
				
				// Set relative differences
				relativeDifferenceX = positionRelativeX - deltaX;
				relativeDifferenceY = positionRelativeY - deltaY;
				relativeDifferenceZ = positionRelativeZ - deltaZ;
				relativeDifferenceE = positionRelativeE - deltaE;
				
				// Set delta ratios
				deltaRatioX = deltaX / distance;
				deltaRatioY = deltaY / distance;
				deltaRatioZ = deltaZ / distance;
				deltaRatioE = deltaE / distance;
				
				// Check if in first dayer and delta E is greater than zero 
				if(firstLayer && deltaE > 0) {
				
					// Check if previous g-code is not empty
					if(!previousGcode.isEmpty()) {
					
						// Check if corner count is at most one and sharp corner
						if(cornerCounter <= 1 && isSharpCornerForWaveBonding(gcode, previousGcode)) {
						
							// Check if refrence g-codes is set
							if(refrenceGcode.isEmpty()) {
							
								// Check if a tack point was created
								tackPoint = createTackPoint(gcode, previousGcode);
								if(!tackPoint.isEmpty())
								
									// Send tack point to temp
									temp << tackPoint << endl; 
							}
							
							// Set refrence g-code
							refrenceGcode = gcode;
							
							// Increment corner counter
							cornerCounter++;
						}
						
						// Otherwise check is corner count is at least one and sharp corner
						else if(cornerCounter >= 1 && isSharpCornerForWaveBonding(gcode, refrenceGcode)) {
						
							// Check if a tack point was created
							tackPoint = createTackPoint(gcode, refrenceGcode);
							if(!tackPoint.isEmpty())
							
								// Send tack point to temp
								temp << tackPoint << endl; 
							
							// Set refrence g-code
							refrenceGcode = gcode;
						}
					}
					
					// Go through all of the wave
					for(uint32_t i = 1; i <= waveRatio; i++) {
					
						// Check if at last component
						if(i == waveRatio) {
						
							// Set temp relative values
							tempRelativeX = positionRelativeX;
							tempRelativeY = positionRelativeY;
							tempRelativeZ = positionRelativeZ;
							tempRelativeE = positionRelativeE;
						}
						
						// Otherwise
						else {
						
							// Set temp relative values
							tempRelativeX = relativeDifferenceX + i * WAVE_PERIOD_QUARTER * deltaRatioX;
							tempRelativeY = relativeDifferenceY + i * WAVE_PERIOD_QUARTER * deltaRatioY;
							tempRelativeZ = relativeDifferenceZ + i * WAVE_PERIOD_QUARTER * deltaRatioZ;
							tempRelativeE = relativeDifferenceE + i * WAVE_PERIOD_QUARTER * deltaRatioE;
						}
						
						// Check if not at least component
						if(i != waveRatio) {
						
							// Set extra g-code G value
							extraGcode.clear();
							extraGcode.setValue('G', gcode.getValue('G'));
							
							// Set extra g-code X value
							if(gcode.hasValue('X'))
								extraGcode.setValue('X', to_string(positionRelativeX - deltaX + tempRelativeX - relativeDifferenceX));
							
							// Set extra g-cdoe Y value
							if(gcode.hasValue('Y'))
								extraGcode.setValue('Y', to_string(positionRelativeY - deltaY + tempRelativeY - relativeDifferenceY));
							
							// Check if plane changed
							if(changesPlane)
							
								// Set extra g-code Z value
								extraGcode.setValue('Z', to_string(positionRelativeZ - deltaZ + tempRelativeZ - relativeDifferenceZ + getCurrentAdjustmentZ()));
							
							// Otherwise check if command has a Z value and changes in Z are noticable
							else if(gcode.hasValue('Z') && deltaZ != DBL_EPSILON)
							
								// Set extra g-code Z value
								extraGcode.setValue('Z', to_string(positionRelativeZ - deltaZ + tempRelativeZ - relativeDifferenceZ));
								
							// Set extra g-code E value
							extraGcode.setValue('E', to_string(positionRelativeE - deltaE + tempRelativeE - relativeDifferenceE));
							// Send extra g-code to temp
							temp << extraGcode << endl;
						}
						
						// Otherwise check if plane changed
						else if(changesPlane) {
						
							// Check if command has a Z value
							if(gcode.hasValue('Z'))
							
								// Add to command's Z value
								gcode.setValue('Z', to_string(stod(gcode.getValue('Z')) + getCurrentAdjustmentZ()));
							
							// Otherwise
							else
							
								// Set command's Z value
								gcode.setValue('Z', to_string(relativeDifferenceZ + deltaZ + getCurrentAdjustmentZ()));
						}
					}
				}
				
				// Set previous gcode
				previousGcode = gcode;
			}
			
			// Otherwise check if command is G90
			else if(gcode.hasValue('G') && gcode.getValue('G') == "90")
				
				// Clear relative mode
				relativeMode = false;
			
			// Otherwise check if command is G91
			else if(gcode.hasValue('G') && gcode.getValue('G') == "91")
				
				// Set relative mode
				relativeMode = true;
			
			// Otherwise check if command is G92
			else if(gcode.hasValue('G') && gcode.getValue('G') == "92") {
			
				// Check if line doesn't contain an X, Y, Z, and E
				if(!gcode.hasValue('X') && !gcode.hasValue('Y') && !gcode.hasValue('Z') && !gcode.hasValue('E')) {
				
					// Set values to zero
					gcode.setValue('X', "0");
					gcode.setValue('Y', "0");
					gcode.setValue('Z', "0");
					gcode.setValue('E', "0");
				}
				
				// Otherwise
				else {
				
					// Set position relative values
					positionRelativeX = !gcode.hasValue('X') ? positionRelativeX : stod(gcode.getValue('X'));
					positionRelativeY = !gcode.hasValue('Y') ? positionRelativeY : stod(gcode.getValue('Y'));
					positionRelativeZ = !gcode.hasValue('Z') ? positionRelativeZ : stod(gcode.getValue('Z'));
					positionRelativeE = !gcode.hasValue('E') ? positionRelativeE : stod(gcode.getValue('E'));
				}
			}
				
			// Send line to temp
			temp << gcode << endl;
		}
		
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

bool thermalBondingPreprocessor() {

	// Initialzie variables
	string line;
	Gcode gcode, previousGcode, refrenceGcode, tackPoint;
	fstream processedFile(folderLocation + "/output.gcode", ios::in | ios::binary);
	fstream temp(folderLocation + "/temp", ios::out | ios::in | ios::binary | ios::app);
	int layerCounter = 0, cornerCounter = 0;
	bool checkSharpCorner = false;
	bool relativeMode = true;
	
	// Check if temp file was opened successfully
	if(processedFile.good() && temp.good()) {
	
		// Go through processed file
		while(processedFile.peek() != EOF) {
	
			// Read in line
			getline(processedFile, line);
			
			// Check if line is a layer command
			if(line.find(";LAYER:") != string::npos) {
			
				// Check how many layers have been processed
				switch(layerCounter) {
			
					// Check if layer counter is zero
					case 0:
				
						// Send temperature command to temp
						temp << "M109 S" << to_string(getBoundedTemperature(temperature + (filament == PLA ? 10 : 15))) << endl;
						// Set check sharp corner
						checkSharpCorner = true;
					break;
				
					// Otherwise check if layer counter is one
					case 1:
				
						// Send temperature command to temp
						temp << "M109 S" << to_string(getBoundedTemperature(temperature + (filament == PLA ? 5 : 10))) << endl;
					break;
				}
				
				// Increment layer counter
				layerCounter++;
			}
			
			// Check if line is layer zero
			if(line.find(";LAYER:0") != string::npos) {
			
				// Send temperature command to temp
				temp << "M109 S" << to_string(temperature) << endl;
				
				// Clear check sharp corner
				checkSharpCorner = false;
			}
			
			// Check if line was parsed successfully and it's a G command and wave bonding is not being used
			if(gcode.parseLine(line) && gcode.hasValue('G') && !useWaveBonding) {
			
				// Check what parameter is associated with the command
				switch(stoi(gcode.getValue('G'))) {
				
					case 0:
					case 1:
					
						// Check if previous command exists, the check sharp corner is set, and filament is ABS, HIPS, or PLA
						if(!previousGcode.isEmpty() && checkSharpCorner && (filament == ABS || filament == HIPS || filament == PLA)) {
							// Check if both counters are less than or equal to one
							if(cornerCounter <= 1 && layerCounter <= 1) {
							
								// Check if sharp corner
								if(isSharpCornerForThermalBonding(gcode, previousGcode)) {
								
									// Check if refrence g-codes is set
									if(refrenceGcode.isEmpty()) {
									
										// Check if a tack point was created
										tackPoint = createTackPoint(gcode, previousGcode);
										if(!tackPoint.isEmpty())
										
											// Send tack point to temp
											temp << tackPoint << endl;
									}
									
									// Set refrence g-code
									refrenceGcode = gcode;
									
									// Increment corner count
									cornerCounter++;
								}
						
							}
							
							// Otherwise check if corner counter is greater than one but layer counter isn't and sharp corner
							else if(cornerCounter >= 1 && layerCounter <= 1 && isSharpCornerForThermalBonding(gcode, refrenceGcode)) {
							
								// Check if a tack point was created
								tackPoint = createTackPoint(gcode, refrenceGcode);
								if(!tackPoint.isEmpty())
								
									// Send tack point to temp
									temp << tackPoint << endl;
								
								
								// Set refrence g-code
								refrenceGcode = gcode;
							}
						}
					break;
					
					case 90:
					
						// Clear relative mode
						relativeMode = false;
					break;
				
					case 91:
					
						// Set relative mode
						relativeMode = true;
					break;
				}
			}
			
			// Set previous g-code
			previousGcode = gcode;
			
			// Check if not using wave bonding, filament is ABS, command contains G and Z, and in absolute mode
			if(!useWaveBonding && filament == ABS && gcode.hasValue('G') && gcode.hasValue('Z') && relativeMode)
				
				// Adjust g-code to have Z lower by height offset
				gcode.setValue('Z', to_string(stod(gcode.getValue('Z')) + BONDING_HEIGHT_OFFSET));
			
			// Send g-code to temp
			temp << gcode << endl;
		}
		
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

bool bedCompensationPreprocessor() {

	// Initialzie variables
	string line;
	Gcode gcode, extraGcode;
	fstream processedFile(folderLocation + "/output.gcode", ios::in | ios::binary);
	fstream temp(folderLocation + "/temp", ios::out | ios::in | ios::binary | ios::app);
	bool relativeMode = true;
	bool changesPlane = false;
	bool hasExtruded = false;
	bool firstLayer = false;
	bool addCommand = false;
	double distance, storedE = 0, heightAdjustment, storedAdjustment, compensationE = 0;
	uint32_t layerNumber = 0, segmentCounter;
	double positionAbsoluteX = 0, positionAbsoluteY = 0, positionAbsoluteZ = 0, positionAbsoluteE = 0;
	double positionRelativeX = 0, positionRelativeY = 0, positionRelativeZ = 0, positionRelativeE = 0;
	double deltaX, deltaY, deltaZ, deltaE;
	double absoluteDifferenceX, absoluteDifferenceY, relativeDifferenceX, relativeDifferenceY, relativeDifferenceZ, relativeDifferenceE;
	double deltaRatioX, deltaRatioY, deltaRatioZ, deltaRatioE;
	double tempAbsoluteX, tempAbsoluteY, tempRelativeX, tempRelativeY, tempRelativeZ, tempRelativeE;
	double leftAdjustment, rightAdjustment;
	
	// Check if temp file was opened successfully
	if(processedFile.good() && temp.good()) {
	
		// Go through processed file
		while(processedFile.peek() != EOF) {
	
			// Read in line
			getline(processedFile, line);
			
			// Check if line was parsed successfully, it's G0 or G1, and it isn't in relative mode
			if(gcode.parseLine(line) && gcode.hasValue('G') && (gcode.getValue('G') == "0" || gcode.getValue('G') == "1") && !relativeMode) {
			
				// Check if command has an X or Y value
				if(gcode.hasValue('X') || gcode.hasValue('X'))
				
					// Set changes plane
					changesPlane = true;
				
				// Check if command contains a Z value
				if(gcode.hasValue('Z'))
				
					// Add to command's Z value
					gcode.setValue('Z', to_string(stod(gcode.getValue('Z')) + printer.getBedHeightOffset()));
				
				// Set delta values
				deltaX = !gcode.hasValue('X') ? 0 : stod(gcode.getValue('X')) - positionRelativeX;
				deltaY = !gcode.hasValue('Y')? 0 : stod(gcode.getValue('Y')) - positionRelativeY;
				deltaZ = !gcode.hasValue('Z') ? 0 : stod(gcode.getValue('Z')) - positionRelativeZ;
				deltaE = !gcode.hasValue('E') ? 0 : stod(gcode.getValue('E')) - positionRelativeE;
				
				// Adjust position absolute and relative values for the changes
				positionAbsoluteX += deltaX;
				positionAbsoluteY += deltaY;
				positionAbsoluteZ += deltaZ;
				positionAbsoluteE += deltaE;
				positionRelativeX += deltaX;
				positionRelativeY += deltaY;
				positionRelativeZ += deltaZ;
				positionRelativeE += deltaE;
				
				// Check if Z has a noticable change
				if(deltaZ != DBL_EPSILON) {
				
					// Set layer number
					layerNumber = hasExtruded ? layerNumber + 1 : 1;
					
					// Set first layer
					firstLayer = layerNumber == 0 || layerNumber == 1;
				}
				
				// Calculate distance
				distance = sqrt(deltaX * deltaX + deltaY * deltaY);
				
				// Set segment counter
				segmentCounter = distance > SEGMENT_LENGTH ? distance / SEGMENT_LENGTH : 1;
				
				// Set absolute and relative differences
				absoluteDifferenceX = positionAbsoluteX - deltaX;
				absoluteDifferenceY = positionAbsoluteY - deltaY;
				relativeDifferenceX = positionRelativeX - deltaX;
				relativeDifferenceY = positionRelativeY - deltaY;
				relativeDifferenceZ = positionRelativeZ - deltaZ;
				relativeDifferenceE = positionRelativeE - deltaE;
				
				// Set delta ratios
				deltaRatioX = deltaX / distance;
				deltaRatioY = deltaY / distance;
				deltaRatioZ = deltaZ / distance;
				deltaRatioE = deltaE / distance;
				
				// Check if change in E is greater than 0
				if(deltaE > 0) {
				
					// Set
					addCommand = !hasExtruded;
					
					// Set has extruded
					hasExtruded = true;
				}
				
				// Check if add command
				if(addCommand) {
				
					// Set extra g-code
					extraGcode.clear();
					extraGcode.setValue('G', "0");
					extraGcode.setValue('E', "0");
					
					// Send extra g-code to temp
					temp << extraGcode << endl;
				}
				
				// Check if layer is targeted and change in E is greater than zero
				if((firstLayer || !FIRST_LAYER_ONLY) && deltaE > 0) {
				
					// Go through all segments
					for (uint32_t i = 1; i <= segmentCounter; i++) {
				
						// Check if at last segment
						if(i == segmentCounter) {
					
							// Set temp values
							tempAbsoluteX = positionAbsoluteX;
							tempAbsoluteY = positionAbsoluteY;
							tempRelativeX = positionRelativeX;
							tempRelativeY = positionRelativeY;
							tempRelativeZ = positionRelativeZ;
							tempRelativeE = positionRelativeE;
						}
				
						// Otherwise
						else {
					
							// Set temp values
							tempAbsoluteX = absoluteDifferenceX + i * SEGMENT_LENGTH * deltaRatioX;
							tempAbsoluteY = absoluteDifferenceY + i * SEGMENT_LENGTH * deltaRatioY;
							tempRelativeX = relativeDifferenceX + i * SEGMENT_LENGTH * deltaRatioX;
							tempRelativeY = relativeDifferenceY + i * SEGMENT_LENGTH * deltaRatioY;
							tempRelativeZ = relativeDifferenceZ + i * SEGMENT_LENGTH * deltaRatioZ;
							tempRelativeE = relativeDifferenceE + i * SEGMENT_LENGTH * deltaRatioE;
						}
						
						// Set height adjustment
						heightAdjustment = getHeightAdjustmentRequired(tempAbsoluteX, tempAbsoluteY);
						
						// Check if set extrusion to compensate
						if(CHANGE_EXTRUSION_TO_COMPENSATE)
						
							// Add value to compensation E
							compensationE += (-heightAdjustment / CHANGE_IN_HEIGHT_THAT_DOUBLES_EXTRUSION) * (tempRelativeE - storedE);
						
						// Store adjustment
						storedAdjustment = heightAdjustment;
						
						// Check if not at last segment
						if(i != segmentCounter) {
						
							// Set extra g-code
							extraGcode.clear();
							extraGcode.setValue('G', gcode.getValue('G'));
							
							// Check if command has an X value
							if(gcode.hasValue('X'))
							
								// Set extra g-code X value
								extraGcode.setValue('X', to_string(positionRelativeX - deltaX + tempRelativeX - relativeDifferenceX));
								
							// Check if command has a Y value
							if(gcode.hasValue('Y'))
							
								// Set extra g-code Y value
								extraGcode.setValue('Y', to_string(positionRelativeY - deltaY + tempRelativeY - relativeDifferenceY));
							
							// Check if set to compensate Z and the plane changed
							if(MOVE_Z_TO_COMPENSATE && changesPlane)
							
								// Set extra g-code Z value
								extraGcode.setValue('Z', to_string(positionRelativeZ - deltaZ + tempRelativeZ - relativeDifferenceZ + storedAdjustment));
							
							// Otherwise check if command has a Z value and the change in Z in noticable
							else if(gcode.hasValue('Z') && deltaZ != DBL_EPSILON)
							
								// Set extra g-code Z value
								extraGcode.setValue('Z', to_string(positionRelativeZ - deltaZ + tempRelativeZ - relativeDifferenceZ));
							
							// Set extra g-gode E value
							extraGcode.setValue('E', to_string(positionRelativeE - deltaE + tempRelativeE - relativeDifferenceE + compensationE));
							
							// Send extra g-code to temp
							temp << extraGcode << endl;
						}
						
						// Otherwise
						else {
						
							// Check if set to compensate Z and the plane changed
							if(MOVE_Z_TO_COMPENSATE && changesPlane) {
							
								// Check if command has a Z value
								if(gcode.hasValue('Z'))
								
									// Add value to command Z value
									gcode.setValue('Z', to_string(stod(gcode.getValue('Z')) + storedAdjustment));
								
								// Otherwise
								else
								
									// Set command Z value
									gcode.setValue('Z', to_string(relativeDifferenceZ + deltaZ + storedAdjustment));
							}
							
							// Check if command has an E value
							if(gcode.hasValue('E'))
							
								// Add value to command E value
								gcode.setValue('E', to_string(stod(gcode.getValue('E')) + compensationE));
						}
						
						// Store relative E
						storedE = tempRelativeE;
					}
				}
				
				// Otherwise
				else {
				
					// Check if set to compensate Z, the plane changed, and layer is targeted
					if(MOVE_Z_TO_COMPENSATE && changesPlane && (firstLayer || !FIRST_LAYER_ONLY)) {
					
						// Set left and right adjustment
						leftAdjustment = (printer.getBackLeftOffset() - printer.getFrontLeftOffset()) / LEVELLING_MOVE_Y * positionAbsoluteY + printer.getFrontLeftOffset();
						rightAdjustment = (printer.getBackRightOffset() - printer.getFrontRightOffset()) / LEVELLING_MOVE_Y * positionAbsoluteY + printer.getFrontRightOffset();
						
						// Set stored adjustment
						storedAdjustment = (rightAdjustment - leftAdjustment) / LEVELLING_MOVE_X * positionAbsoluteX + leftAdjustment;
						
						// Check if command has a Z value
						if(gcode.hasValue('Z'))
						
							// Add value to command Z
							gcode.setValue('Z', to_string(stod(gcode.getValue('Z')) + storedAdjustment));
						
						// Otherwise
						else
						
							// Set command Z
							gcode.setValue('Z', to_string(positionRelativeZ + storedAdjustment));
					}
					
					// Check if command has an E value
					if(gcode.hasValue('E'))
					
						// Add value to command E value
						gcode.setValue('E', to_string(stod(gcode.getValue('E')) + compensationE));
					
					// Store relative E
					storedE = positionRelativeE;
				}
			}
			
      			// Otherwise check if command is G90
			else if(gcode.hasValue('G') && gcode.getValue('G') == "90")
				
				// Clear relative mode
				relativeMode = false;
			
			// Otherwise check if command is G91
			else if(gcode.hasValue('G') && gcode.getValue('G') == "91")
				
				// Set relative mode
				relativeMode = true;
			
			// Otherwise check if command is G92
			else if(gcode.hasValue('G') && gcode.getValue('G') == "92") {
			
				// Check if command doesn't have an X, Y, Z, and E value
				if(!gcode.hasValue('X') && !gcode.hasValue('Y') && !gcode.hasValue('Z') && !gcode.hasValue('E')) {
			
					// Set command values to zero
					gcode.setValue('X', "0");
					gcode.setValue('Y', "0");
					gcode.setValue('Z', "0");
					gcode.setValue('E', "0");
				}
			
				// Otherwise
				else {
			
					// Set relative positions
					positionRelativeX = !gcode.hasValue('X') ? positionRelativeX : stod(gcode.getValue('X'));
					positionRelativeY = !gcode.hasValue('Y') ? positionRelativeY : stod(gcode.getValue('Y'));
					positionRelativeZ = !gcode.hasValue('Z') ? positionRelativeZ : stod(gcode.getValue('Z'));
					positionRelativeE = !gcode.hasValue('E') ? positionRelativeE : stod(gcode.getValue('E'));
				}
			}
				
			// Send line to temp
			temp << gcode << endl;
		}
		
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

bool backlashCompensationPreprocessor() {

	// Initialzie variables
	string line;
	Gcode gcode, extraGcode;
	fstream processedFile(folderLocation + "/output.gcode", ios::in | ios::binary);
	fstream temp(folderLocation + "/temp", ios::out | ios::in | ios::binary | ios::app);
	bool relativeMode = true;
	string valueF = "2000";
	direction directionX, directionY, previousDirectionX = NEITHER, previousDirectionY = NEITHER;
	double compensationX = 0, compensationY = 0;
	double positionRelativeX = 0, positionRelativeY = 0, positionRelativeZ = 0, positionRelativeE = 0;
	double deltaX, deltaY, deltaZ, deltaE;
	
	// Check if temp file was opened successfully
	if(processedFile.good() && temp.good()) {
	
		// Go through processed file
		while(processedFile.peek() != EOF) {
	
			// Read in line
			getline(processedFile, line);
			
			// Check if line was parsed successfully, it's G0 or G1, and it isn't in relative mode
			if(gcode.parseLine(line) && gcode.hasValue('G') && (gcode.getValue('G') == "0" || gcode.getValue('G') == "1") && !relativeMode) {
			
				// Check if command has an F value
				if(gcode.hasValue('F'))
			
					// Set value F
					valueF = gcode.getValue('F');
				
				// Set delta values
				deltaX = !gcode.hasValue('X') ? 0 : stod(gcode.getValue('X')) - positionRelativeX;
				deltaY = !gcode.hasValue('Y') ? 0 : stod(gcode.getValue('Y')) - positionRelativeY;
				deltaZ = !gcode.hasValue('Z') ? 0 : stod(gcode.getValue('Z')) - positionRelativeZ;
				deltaE = !gcode.hasValue('E') ? 0 : stod(gcode.getValue('E')) - positionRelativeE;
				
				// Set directions
				directionX = deltaX > DBL_EPSILON ? POSITIVE : deltaX < -DBL_EPSILON ? NEGATIVE : previousDirectionX;
				directionY = deltaY > DBL_EPSILON ? POSITIVE : deltaY < -DBL_EPSILON ? NEGATIVE : previousDirectionY;
				
				// Check if direction has changed
				if((directionX != previousDirectionX && previousDirectionX != NEITHER) || (directionY != previousDirectionY && previousDirectionY != NEITHER)) {
				
					// Set extra g-code G value
					extraGcode.clear();
					extraGcode.setValue('G', gcode.getValue('G'));
					
					// Check if X direction has changed
					if(directionX != previousDirectionX && previousDirectionX != NEITHER) {
					
						// Set X compensation
						compensationX += printer.getBacklashX() * (directionX == POSITIVE ? 1 : -1);
						
						// Set extra g-code X value
						extraGcode.setValue('X', to_string(positionRelativeX + compensationX));
					}
					
					// Check if Y direction has changed
					if(directionY != previousDirectionY && previousDirectionY != NEITHER) {
					
						// Set Y compensation
						compensationY += printer.getBacklashY() * (directionY == POSITIVE ? 1 : -1);
						
						// Set extra g-code Y value
						extraGcode.setValue('Y', to_string(positionRelativeY + compensationY));
					}
					
					// Set extra g-code F value
					extraGcode.setValue('F', "2900");
					
					// Send extra g-code to temp
					temp << extraGcode << endl;
					
					// Set command's F value
					gcode.setValue('F', valueF);
				}
			
				// Check if command has an X value
				if(gcode.hasValue('X'))
			
					// Add to command's X value
					gcode.setValue('X', to_string(stod(gcode.getValue('X')) + compensationX));
			
				// Check if command has a Y value
				if(gcode.hasValue('Y'))
			
					// Add to command's Y value
					gcode.setValue('Y', to_string(stod(gcode.getValue('Y')) + compensationY));
			
				// Set relative values
				positionRelativeX += deltaX;
				positionRelativeY += deltaY;
				positionRelativeZ += deltaZ;
				positionRelativeE += deltaE;
				
				// Store directions
				previousDirectionX = directionX;
				previousDirectionY = directionY;
			}
			
      			// Otherwise check if command is G90
			else if(gcode.hasValue('G') && gcode.getValue('G') == "90")
				
				// Clear relative mode
				relativeMode = false;
			
			// Otherwise check if command is G91
			else if(gcode.hasValue('G') && gcode.getValue('G') == "91")
				
				// Set relative mode
				relativeMode = true;
			
			// Otherwise check if command is G92
			else if(gcode.hasValue('G') && gcode.getValue('G') == "92") {
			
				// Check if command doesn't have an X, Y, Z, and E value
				if(!gcode.hasValue('X') && !gcode.hasValue('Y') && !gcode.hasValue('Z') && !gcode.hasValue('E')) {
			
					// Set command values to zero
					gcode.setValue('X', "0");
					gcode.setValue('Y', "0");
					gcode.setValue('Z', "0");
					gcode.setValue('E', "0");
				}
			
				// Otherwise
				else {
			
					// Set relative positions
					positionRelativeX = !gcode.hasValue('X') ? positionRelativeX : stod(gcode.getValue('X'));
					positionRelativeY = !gcode.hasValue('Y') ? positionRelativeY : stod(gcode.getValue('Y'));
					positionRelativeZ = !gcode.hasValue('Z') ? positionRelativeZ : stod(gcode.getValue('Z'));
					positionRelativeE = !gcode.hasValue('E') ? positionRelativeE : stod(gcode.getValue('E'));
				}
			}
				
			// Send line to temp
			temp << gcode << endl;
		}
		
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

bool feedRateConversionPreprocessor() {

	// Initialzie variables
	string line;
	Gcode gcode;
	fstream processedFile(folderLocation + "/output.gcode", ios::in | ios::binary);
	fstream temp(folderLocation + "/temp", ios::out | ios::in | ios::binary | ios::app);
	double commandFeedRate;
	
	// Check if temp file was opened successfully
	if(processedFile.good() && temp.good()) {
	
		// Go through processed file
		while(processedFile.peek() != EOF) {
	
			// Read in line
			getline(processedFile, line);
			
			// Check if line was parsed successfully and it contains G and F values
			if(gcode.parseLine(line) && gcode.hasValue('G') && gcode.hasValue('F')) {
			
				// Get command's feedrate
				commandFeedRate = stod(gcode.getValue('F')) / 60;
				
				// Force feed rate to adhere to limitations
				if(commandFeedRate > MAX_FEED_RATE)
                			commandFeedRate = MAX_FEED_RATE;
                		
                		// Calculate adjusted feed rate
                		commandFeedRate = 30 + (1 - commandFeedRate / MAX_FEED_RATE) * 800;
                		
				// Set new feed rate for the command
				gcode.setValue('F', to_string(commandFeedRate));
			}
				
			// Send line to temp
			temp << gcode << endl;
		}
		
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
