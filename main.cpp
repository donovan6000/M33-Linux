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

bool useBasicPreparation = false;
bool useWaveBonding = false;
bool useThermalBonding = false;
bool useBedCompensation = false;
bool useBacklashCompensation = false;
bool useFeedRateConversion = false;

#define WAVE_PERIOD_QUARTER 1.25
#define WAVE_SIZE 0.15;

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
	cout << "M3D Linux V0.4" << endl;
	
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
					response = printer.receiveResponse();
					if(line.substr(0, 2) == "G0" && response == "Info:Too small")
						response = printer.receiveResponse();
					
					// Display response
					cout << "Receive: " << response << endl << endl;
				} while(response.substr(0, 2) != "ok");
			
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
				
				// Get next command if line didn't contain valid g-code
				if(!printer.sendRequest(line)) {
				
					cout << "Failed to parse command" << endl << endl;
					break;
				}
				
				// Display command
				cout << "Send: " << line << endl;
				
				// Get valid response
				response = printer.receiveResponse();
				if(line.substr(0, 2) == "G0" && response == "Info:Too small")
					response = printer.receiveResponse();
				
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
	bool relativeMode = false, positiveExtrusion;
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
		
		// Return if print doesn't go out of bounds
		
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

	// Return distance between the two values
	return sqrt(pow(stod(firstPoint.getValue('X')) - stod(secondPoint.getValue('X')), 2) + pow(stod(firstPoint.getValue('Y')) - stod(secondPoint.getValue('Y')), 2));
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
	double currentX = stod(point.getValue('X'));
	double currentY = stod(point.getValue('Y'));
	double previousX = stod(refrence.getValue('X'));
	double previousY = stod(refrence.getValue('Y'));
	
	// Calculate value
	double value = acos((currentX * previousX + currentY * previousY) / (pow(currentX * currentX + currentY * currentY, 2) * pow(previousX * previousX + previousY * previousY, 2)));
	
	// Return if sharp corner
	return value > 0 && value < M_PI_2;
}

bool isSharpCornerForWaveBonding(const Gcode &point, const Gcode &refrence) {

	// Initialize variables
	double currentX = stod(point.getValue('X'));
	double currentY = stod(point.getValue('Y'));
	double previousX = stod(refrence.getValue('X'));
	double previousY = stod(refrence.getValue('Y'));
	
	// Calculate value
	double value = acos((currentX * previousX + currentY + previousY) / (pow(currentX * currentX + currentY + currentY, 2) * pow(previousX * previousX + previousY + previousY, 2)));
	
	// Return if sharp corner
	return value > 0 && value < M_PI_2;
}

/*void ProcessForTackPoints(GCodeWriter output_writer, const GCode &point, const GCode &refrence, GCode &lastTackPoint, int cornercount) {

	// Initialize variables
	Gcode tackPoint;
	if(cornercount <= 1 && isSharpCornerForWave(point, refrence)) {
		if(lastTackPoint.empty()) {
			tackPoint = createTackPoint(point, refrence);
			if(!tackPoint.empty())
				temp << tackPoint << endl; 
		}
		lastTackPoint = point;
		cornercount++;
	}
	else if(cornercount >= 1 && isSharpCornerForWave(point, lastTackPoint)) {
		tackPoint = createTackPoint(point, lastTackPoint);
		if(!tackPoint.empty())
			temp << tackPoint << endl; 
		lastTackPoint = point;
	}
}*/

float getCurrentAdjustmentZ() {

	// Initialize variables
	static uint8_t waveStep = 0;

	// Set adjustment
	float adjustment = waveStep ? waveStep != 2 ? 0 : -1.5 : 1;
	
	// Increment wave step
	waveStep = (waveStep + 1) % 4;
	
	// Return adjustment
	return adjustment * WAVE_SIZE;
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
	Gcode gcode, previousGcode, refrenceGcode, tackPoint;
	fstream processedFile(folderLocation + "/output.gcode", ios::in | ios::binary);
	fstream temp(folderLocation + "/temp", ios::out | ios::in | ios::binary | ios::app);
	int num = 0;
	bool relativeMode = true;
	bool flag2 = true;
	bool flag3 = false;
	//Position position = new Position();
	float num5 = 0;
	int cornercount = 0;
	
	// Check if temp file was opened successfully
	if(processedFile.good() && temp.good()) {
	
		// Go through processed file
		while(processedFile.peek() != EOF) {
	
			// Read in line
			getline(processedFile, line);
			
			// Check if line is a layer command
			if(line.find(";LAYER:") != string::npos) {
				int num7 = stoi(line.substr(7));
				if (num7 < num)
				num = num7;
				flag2 = num7 == num;
			}
			
			if(gcode.parseLine(line) && (gcode.getValue('G') == "0" || gcode.getValue('G') == "1") && !relativeMode) {
				/*if (currLine.hasX || currLine.hasY)
					flag3 = true;
				if (currLine.hasZ && flag2)
					currLine.Z += -0.1f;
					
				float num8 = !currLine.hasX ? 0f : (currLine.X - position.relativeX);
				float num9 = !currLine.hasY ? 0f : (currLine.Y - position.relativeY);
				float num10 = !currLine.hasZ ? 0f : (currLine.Z - position.relativeZ);
				float num11 = !currLine.hasE ? 0f : (currLine.E - position.relativeE);
				position.absoluteX += num8;
				position.absoluteY += num9;
				position.absoluteZ += num10;
				position.absoluteE += num11;
				position.relativeX += num8;
				position.relativeY += num9;
				position.relativeZ += num10;
				position.relativeE += num11;
				if (currLine.hasF)
					position.F = currLine.F;
				float num12 = (float) Math.Sqrt((double) ((num8 * num8) + (num9 * num9)));
				int num13 = 1;
				if (num12 > WAVE_PERIOD_QUARTER)
					num13 = (int) (num12 / WAVE_PERIOD_QUARTER);
				float num14 = position.absoluteX - num8;
				float num15 = position.absoluteY - num9;
				float num16 = position.relativeX - num8;
				float num17 = position.relativeY - num9;
				float num18 = position.relativeZ - num10;
				float num19 = position.relativeE - num11;
				float num20 = num8 / num12;
				float num21 = num9 / num12;
				float num22 = num10 / num12;
				float num23 = num11 / num12;
				if (flag2 && (num11 > 0f)) {
					if (prevLine != null)
						this.ProcessForTackPoints(output_writer, currLine, prevLine, ref code3, ref cornercount);
					for (int i = 1; i < (num13 + 1); i++) {
						float relativeX;
						float relativeY;
						float relativeZ;
						float relativeE;
						if (i == num13) {
							float absoluteX = position.absoluteX;
							float absoluteY = position.absoluteY;
							relativeX = position.relativeX;
							relativeY = position.relativeY;
							relativeZ = position.relativeZ;
							relativeE = position.relativeE;
						}
						else {
							relativeX = num16 + ((i * WAVE_PERIOD_QUARTER) * num20);
							relativeY = num17 + ((i * WAVE_PERIOD_QUARTER) * num21);
							relativeZ = num18 + ((i * WAVE_PERIOD_QUARTER) * num22);
							relativeE = num19 + ((i * WAVE_PERIOD_QUARTER) * num23);
						}
						float num29 = relativeE - num5;
						if (i != num13) {
							GCode code = new GCode {
								G = currLine.G
							};
							if (currLine.hasX)
								code.X = (position.relativeX - num8) + (relativeX - num16);
							if (currLine.hasY)
								code.Y = (position.relativeY - num9) + (relativeY - num17);
							if (flag3)
								code.Z = ((position.relativeZ - num10) + (relativeZ - num18)) + this.CurrentAdjustmentsZ;
							else if (currLine.hasZ && ((num10 > float.Epsilon) || (num10 < -1.401298E-45f)))
								code.Z = (position.relativeZ - num10) + (relativeZ - num18);
						
							code.E = ((position.relativeE - num11) + (relativeE - num19));
							output_writer.Write(code);
						}
						else {
							if (flag3) {
								if (currLine.hasZ)
									currLine.Z += this.CurrentAdjustmentsZ;
								else
									currLine.Z = (num18 + num10) + this.CurrentAdjustmentsZ;
							}
						}
						num5 = relativeE;
					}
				}*/
				
				// Set previous gcode
				previousGcode = gcode;
			}
			
			// Otherwise check if command is G90
			else if(gcode.getValue('G') == "90")
				
				// Clear relative mode
				relativeMode = false;
			
			// Otherwise check if command is G91
			else if(gcode.getValue('G') == "91")
				
				// Set relative mode
				relativeMode = true;
			
			// Otherwise check if command is G92
			else if(gcode.getValue('G') == "92") {
			
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
					/*position.relativeX = !gcode.hasValue('X') ? position.relativeX : gcode.getValue('X');
					position.relativeY = !gcode.hasValue('Y') ? position.relativeY : gcode.getValue('Y');
					position.relativeZ = !gcode.hasValue('Z') ? position.relativeZ : gcode.getValue('Z');
					position.relativeE = !gcode.hasValue('E') ? position.relativeE : gcode.getValue('E');*/
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
	bool relativeMode = false;
	
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
				
				// Adjust g-code to have Z lower by 0.1
				gcode.setValue('Z', to_string(stod(gcode.getValue('Z')) - 0.1));
			
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
	Gcode gcode;
	fstream processedFile(folderLocation + "/output.gcode", ios::in | ios::binary);
	fstream temp(folderLocation + "/temp", ios::out | ios::in | ios::binary | ios::app);
	
	// Check if temp file was opened successfully
	if(processedFile.good() && temp.good()) {
	
		// Go through processed file
		while(processedFile.peek() != EOF) {
	
			// Read in line
			getline(processedFile, line);
			
			// Check if line was parsed successfully
			gcode.parseLine(line);
				
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
	Gcode gcode;
	fstream processedFile(folderLocation + "/output.gcode", ios::in | ios::binary);
	fstream temp(folderLocation + "/temp", ios::out | ios::in | ios::binary | ios::app);
	
	// Check if temp file was opened successfully
	if(processedFile.good() && temp.good()) {
	
		// Go through processed file
		while(processedFile.peek() != EOF) {
	
			// Read in line
			getline(processedFile, line);
			
			// Check if line was parsed successfully
			gcode.parseLine(line);
				
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
	const double maxFeedRatePerSecond = 60.0001;
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
				if(commandFeedRate > maxFeedRatePerSecond)
                			commandFeedRate = maxFeedRatePerSecond;
                		
                		// Calculate adjusted feed rate
                		commandFeedRate = 30 + (1 - commandFeedRate / maxFeedRatePerSecond) * 800;
                		
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
