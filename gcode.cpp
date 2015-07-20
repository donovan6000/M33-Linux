// Header files
#include <cstring>
#include "gcode.h"


// Supporting function implementation
Gcode::Gcode() {

	// Set parameter value size
	parameterValue.resize(16);
	
	// Set inital data type
	dataType = 0x1080;
	
	// Clear parsed
	parsed = false;
	
	// Set empty
	empty = true;
}

Gcode::Gcode(Gcode &value) {

	// Copy data type
	value.dataType = this->dataType;
	
	// Copy host command
	value.hostCommand = this->hostCommand;
	
	// Copy parameter values
	value.parameterValue = this->parameterValue;
	
	// Copy parsed
	value.parsed = this->parsed;
	
	// Copy original command
	value.originalCommand = this->originalCommand;
	
	// Copy empty
	value.empty = this->empty;
}

bool Gcode::parseLine(const char *line) {

	// Initialize variables
	char parameterIdentifier = 0;
	string currentValue;
	char *commandStart;
	
	// Reset parameter values
	parameterValue.clear();
	parameterValue.resize(16);
	
	// Reset data type
	dataType = 0x1080;
	
	// Reset parsed
	parsed = false;
	
	// Clear empty
	empty = false;
	
	// Clear host command
	hostCommand.clear();
	
	// Skip leading whitespace
	for(commandStart = const_cast<char *>(line); strlen(commandStart) && (*commandStart == ' ' || *commandStart == '\t' || *commandStart == '\r' || *commandStart == '\n'); commandStart++);
	
	// Set original command
	originalCommand = commandStart;
	
	// Remove trailing whitespace from original command
	while(!originalCommand.empty() && (originalCommand.back() == ' ' || originalCommand.back() == '\t' || originalCommand.back() == '\r' || originalCommand.back() == '\n'))
		originalCommand.pop_back();
	
	// Check if host command
	if(commandStart[0] == '@') {
	
		// Set host command
		hostCommand = commandStart;
		
		// Remove trailing comment if it exists
		if(hostCommand.find(";") != string::npos)
			hostCommand.erase(hostCommand.find(";"));
		
		// Remove trailing whitespace
		while(hostCommand.back() == ' ' || hostCommand.back() == '\t' || hostCommand.back() == '\r' || hostCommand.back() == '\n')
			hostCommand.pop_back();
		
		// Set parsed
		parsed = true;
	
		// Return true
		return true;
	}

	// Go through data
	for(uint8_t i = 0; i <= strlen(commandStart); i++) {
	
		// Check if a parameter is detected
		if(i == 0 || (commandStart[i] >= 'A' && commandStart[i] <= 'Z') || commandStart[i] == ';' || commandStart[i] == '*' || commandStart[i] == ' ' || !commandStart[i]) {
		
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
				for(; i < strlen(commandStart) && commandStart[i] != ';' && commandStart[i] != '\r' && commandStart[i] != '\n'; i++)
					currentValue.push_back(commandStart[i]);
				
				// Check if a string exists
				if(!currentValue.empty()) {
				
					// Set data type
					dataType |= (1 << 15);
				
					// Store parameter value
					parameterValue[15] = currentValue;
				}
			}
			
			// Check if a comment or checksum is detected
			if(commandStart[i] == ';' || commandStart[i] == '*')
		
				// Stop parsing
				break;
			
			// Set parameter identifier
			parameterIdentifier = commandStart[i];
		}
	
		// Otherwise check if value isn't whitespace
		else if(commandStart[i] != ' ' && commandStart[i] != '\t' && commandStart[i] != '\r' && commandStart[i] != '\n')
	
			// Get current value
			currentValue.push_back(commandStart[i]);
	}
	
	// Return if data wasn't empty and set parsed
	return parsed = dataType != 0x1080;
}

bool Gcode::parseLine(const string &line) {

	// Return if line was successfully parsed
	return parseLine(line.c_str());
}

vector<uint8_t> Gcode::getBinary() const {

	// Initialize variables
	vector<uint8_t> request(4);
	uint16_t sum1 = 0, sum2 = 0;
	int32_t tempNumber, *tempPointer;
	float tempFloat;
	
	// Check if host command
	if(!hostCommand.empty()) {
	
		// Set request to host command
		request.resize(0);
		for(uint8_t i = 0; i < hostCommand.length(); i++)
			request.push_back(hostCommand[i]);
	}
	
	// Otherwise
	else {
		
		// Fill first four bytes of request to data type
		request[0] = dataType;
		request[1] = dataType >> 8;
		request[2] = dataType >> 16;
		request[3] = dataType >> 24;
	
		// Check if string parameter is set
		if(dataType & (1 << 15))
	
			// Set fifth byte of request to string length
			request.push_back(parameterValue[15].size());
		
		// Check if command contains N and a value
		if(dataType & 1 && !parameterValue[0].empty()) {
		
			// Set 2 byte integer parameter value
			tempNumber = stoi(parameterValue[0]);
			request.push_back(tempNumber & 0xFF);
			request.push_back((tempNumber >> 8) & 0xFF);
		}
		
		// Check if command contains M and a value
		if(dataType & (1 << 1) && !parameterValue[1].empty()) {
		
			// Set 2 byte integer parameter value
			tempNumber = stoi(parameterValue[1]);
			request.push_back(tempNumber & 0xFF);
			request.push_back((tempNumber >> 8) & 0xFF);
		}
		
		// Check if command contains G and a value
		if(dataType & (1 << 2) && !parameterValue[2].empty()) {
		
			// Set 2 byte integer parameter value
			tempNumber = stoi(parameterValue[2]);
			request.push_back(tempNumber & 0xFF);
			request.push_back((tempNumber >> 8) & 0xFF);
		}
		
		// Check if command contains X and a value
		if(dataType & (1 << 3) && !parameterValue[3].empty()) {
			
			// Set 4 byte float parameter value
			tempFloat = stof(parameterValue[3]);
			tempPointer = reinterpret_cast<int32_t *>(&tempFloat);
			request.push_back(*tempPointer & 0xFF);
			request.push_back((*tempPointer >> 8) & 0xFF);
			request.push_back((*tempPointer >> 16) & 0xFF);
			request.push_back((*tempPointer >> 24) & 0xFF);
		}
		
		// Check if command contains Y and a value
		if(dataType & (1 << 4) && !parameterValue[4].empty()) {
			
			// Set 4 byte float parameter value
			tempFloat = stof(parameterValue[4]);
			tempPointer = reinterpret_cast<int32_t *>(&tempFloat);
			request.push_back(*tempPointer & 0xFF);
			request.push_back((*tempPointer >> 8) & 0xFF);
			request.push_back((*tempPointer >> 16) & 0xFF);
			request.push_back((*tempPointer >> 24) & 0xFF);
		}
		
		// Check if command contains Z and a value
		if(dataType & (1 << 5) && !parameterValue[5].empty()) {
			
			// Set 4 byte float parameter value
			tempFloat = stof(parameterValue[5]);
			tempPointer = reinterpret_cast<int32_t *>(&tempFloat);
			request.push_back(*tempPointer & 0xFF);
			request.push_back((*tempPointer >> 8) & 0xFF);
			request.push_back((*tempPointer >> 16) & 0xFF);
			request.push_back((*tempPointer >> 24) & 0xFF);
		}
		
		// Check if command contains E and a value
		if(dataType & (1 << 6) && !parameterValue[6].empty()) {
			
			// Set 4 byte float parameter value
			tempFloat = stof(parameterValue[6]);
			tempPointer = reinterpret_cast<int32_t *>(&tempFloat);
			request.push_back(*tempPointer & 0xFF);
			request.push_back((*tempPointer >> 8) & 0xFF);
			request.push_back((*tempPointer >> 16) & 0xFF);
			request.push_back((*tempPointer >> 24) & 0xFF);
		}
		
		// Check if command contains F and a value
		if(dataType & (1 << 8) && !parameterValue[7].empty()) {
			
			// Set 4 byte float parameter value
			tempFloat = stof(parameterValue[7]);
			tempPointer = reinterpret_cast<int32_t *>(&tempFloat);
			request.push_back(*tempPointer & 0xFF);
			request.push_back((*tempPointer >> 8) & 0xFF);
			request.push_back((*tempPointer >> 16) & 0xFF);
			request.push_back((*tempPointer >> 24) & 0xFF);
		}
		
		// Check if command contains T and a value
		if(dataType & (1 << 9) && !parameterValue[8].empty()) {
		
			// Set 1 byte integer parameter value
			tempNumber = stoi(parameterValue[8]);
			request.push_back(tempNumber & 0xFF);
		}
		
		// Check if command contains S and a value
		if(dataType & (1 << 10) && !parameterValue[9].empty()) {
			
			// Set 4 byte integer parameter value
			tempNumber = stoi(parameterValue[9]);
			request.push_back(tempNumber & 0xFF);
			request.push_back((tempNumber >> 8) & 0xFF);
			request.push_back((tempNumber >> 16) & 0xFF);
			request.push_back((tempNumber >> 24) & 0xFF);
		}
		
		// Check if command contains P and a value
		if(dataType & (1 << 11) && !parameterValue[10].empty()) {
			
			// Set 4 byte integer parameter value
			tempNumber = stoi(parameterValue[10]);
			request.push_back(tempNumber & 0xFF);
			request.push_back((tempNumber >> 8) & 0xFF);
			request.push_back((tempNumber >> 16) & 0xFF);
			request.push_back((tempNumber >> 24) & 0xFF);
		}
		
		// Check if command contains I and a value
		if(dataType & (1 << 16) && !parameterValue[11].empty()) {
			
			// Set 4 byte float parameter value
			tempFloat = stof(parameterValue[11]);
			tempPointer = reinterpret_cast<int32_t *>(&tempFloat);
			request.push_back(*tempPointer & 0xFF);
			request.push_back((*tempPointer >> 8) & 0xFF);
			request.push_back((*tempPointer >> 16) & 0xFF);
			request.push_back((*tempPointer >> 24) & 0xFF);
		}
		
		// Check if command contains J and a value
		if(dataType & (1 << 17) && !parameterValue[12].empty()) {
			
			// Set 4 byte float parameter value
			tempFloat = stof(parameterValue[12]);
			tempPointer = reinterpret_cast<int32_t *>(&tempFloat);
			request.push_back(*tempPointer & 0xFF);
			request.push_back((*tempPointer >> 8) & 0xFF);
			request.push_back((*tempPointer >> 16) & 0xFF);
			request.push_back((*tempPointer >> 24) & 0xFF);
		}
		
		// Check if command contains R and a value
		if(dataType & (1 << 18) && !parameterValue[13].empty()) {
			
			// Set 4 byte float parameter value
			tempFloat = stof(parameterValue[13]);
			tempPointer = reinterpret_cast<int32_t *>(&tempFloat);
			request.push_back(*tempPointer & 0xFF);
			request.push_back((*tempPointer >> 8) & 0xFF);
			request.push_back((*tempPointer >> 16) & 0xFF);
			request.push_back((*tempPointer >> 24) & 0xFF);
		}
		
		// Check if command contains D and a value
		if(dataType & (1 << 19) && !parameterValue[14].empty()) {
			
			// Set 4 byte float parameter value
			tempFloat = stof(parameterValue[14]);
			tempPointer = reinterpret_cast<int32_t *>(&tempFloat);
			request.push_back(*tempPointer & 0xFF);
			request.push_back((*tempPointer >> 8) & 0xFF);
			request.push_back((*tempPointer >> 16) & 0xFF);
			request.push_back((*tempPointer >> 24) & 0xFF);
		}
		
		// Check if command contains a string
		if(dataType & (1 << 15))
		
			// Set string parameter value
			for(uint8_t i = 0; i < parameterValue[15].length(); i++)
				request.push_back(parameterValue[15][i]);
		
		// Go through all values
		for(uint8_t index = 0; index < request.size(); index++) {

			// Set sums
			sum1 = (sum1 + request[index]) % 0xFF;
			sum2 = (sum1 + sum2)  % 0xFF;
		}

		// Append Fletcher 16 checksum checksum to request
		request.push_back(sum1);
		request.push_back(sum2);
	}
	
	// Return request
	return request;
}

string Gcode::getAscii() const {

	// Initialize variables
	string request;
	
	// Check if host command
	if(!hostCommand.empty())
	
		// Return host command
		return hostCommand;
	
	// Check if command contains N
	if(dataType & 1)
		
		// Append parameter identifier and value
		request += 'N' + parameterValue[0] + ' ';
	
	// Check if command contains M
	if(dataType & (1 << 1)) {
		
		// Append parameter identifier and value
		request += 'M' + parameterValue[1] + ' ';
		
		// Check if command contains a string
		if(dataType & (1 << 15))
		
			// Append string to request
			request += parameterValue[15] + ' ';
	}
	
	// Check if command contains G
	if(dataType & (1 << 2))
		
		// Append parameter identifier and value
		request += 'G' + parameterValue[2] + ' ';
	
	// Check if command contains X
	if(dataType & (1 << 3))
		
		// Append parameter identifier and value
		request += 'X' + parameterValue[3] + ' ';
	
	// Check if command contains Y
	if(dataType & (1 << 4))
		
		// Append parameter identifier and value
		request += 'Y' + parameterValue[4] + ' ';
	
	// Check if command contains Z
	if(dataType & (1 << 5))
		
		// Append parameter identifier and value
		request += 'Z' + parameterValue[5] + ' ';
	
	// Check if command contains E
	if(dataType & (1 << 6))
		
		// Append parameter identifier and value
		request += 'E' + parameterValue[6] + ' ';
	
	// Check if command contains F
	if(dataType & (1 << 8))
		
		// Append parameter identifier and value
		request += 'F' + parameterValue[7] + ' ';
	
	// Check if command contains T
	if(dataType & (1 << 9))
		
		// Append parameter identifier and value
		request += 'T' + parameterValue[8] + ' ';
	
	// Check if command contains S
	if(dataType & (1 << 10))
		
		// Append parameter identifier and value
		request += 'S' + parameterValue[9] + ' ';
	
	// Check if command contains P
	if(dataType & (1 << 11))
		
		// Append parameter identifier and value
		request += 'P' + parameterValue[10] + ' ';
	
	// Check if command contains I
	if(dataType & (1 << 16))
		
		// Append parameter identifier and value
		request += 'I' + parameterValue[11] + ' ';
	
	// Check if command contains J
	if(dataType & (1 << 17))
		
		// Append parameter identifier and value
		request += 'J' + parameterValue[12] + ' ';
	
	// Check if command contains R
	if(dataType & (1 << 18))
		
		// Append parameter identifier and value
		request += 'R' + parameterValue[13] + ' ';
	
	// Check if command contains D
	if(dataType & (1 << 19))
		
		// Append parameter identifier and value
		request += 'D' + parameterValue[14] + ' ';

	// Remove last space from request
	if(!request.empty())
		request.pop_back();
	
	// Return request
	return request;
}

uint32_t Gcode::getDataType() const {

	// Return data type
	return dataType;
}

bool Gcode::hasParameter(char identifier) const {

	// Check what parameter is requested
	switch(identifier) {
		
		case 'N':
		
			// Return if value is set
			return dataType & 1;
		break;
		
		case 'M':
		
			// Return if value is set
			return dataType & (1 << 1);
		break;
		
		case 'G':
		
			// Return if value is set
			return dataType & (1 << 2);
		break;
		
		case 'X':
		
			// Return if value is set
			return dataType & (1 << 3);
		break;
		
		case 'Y':
		
			// Return if value is set
			return dataType & (1 << 4);
		break;
		
		case 'Z':
		
			// Return if value is set
			return dataType & (1 << 5);
		break;
		
		case 'E':
		
			// Return if value is set
			return dataType & (1 << 6);
		break;
		
		case 'F':
		
			// Return if value is set
			return dataType & (1 << 8);
		break;
		
		case 'T':
		
			// Return if value is set
			return dataType & (1 << 9);
		break;
		
		case 'S':
		
			// Return if value is set
			return dataType & (1 << 10);
		break;
		
		case 'P':
		
			// Return if value is set
			return dataType & (1 << 11);
		break;
		
		case 'I':
		
			// Return if value is set
			return dataType & (1 << 16);
		break;
		
		case 'J':
		
			// Return if value is set
			return dataType & (1 << 17);
		break;
		
		case 'R':
		
			// Return if value is set
			return dataType & (1 << 18);
		break;
		
		case 'D':
		
			// Return if value is set
			return dataType & (1 << 19);
		break;
	}
	
	// Return false
	return false;
}

void Gcode::removeParameter(char identifier) {

	// Check what parameter is set
	switch(identifier) {
		
		case 'N':
		
			// Clear data type
			dataType &= ~1;
		
			// Clear parameter value
			parameterValue[0].clear();
		break;
		
		case 'M':
		
			// Clear data type
			dataType &= ~(1 << 1);
		
			// Clear parameter value
			parameterValue[1].clear();
		break;
		
		case 'G':
		
			// Clear data type
			dataType &= ~(1 << 2);
		
			// Clear parameter value
			parameterValue[2].clear();
		break;
		
		case 'X':
		
			// Clear data type
			dataType &= ~(1 << 3);
		
			// Clear parameter value
			parameterValue[3].clear();
		break;
		
		case 'Y':
		
			// Clear data type
			dataType &= ~(1 << 4);
		
			// Clear parameter value
			parameterValue[4].clear();
		break;
		
		case 'Z':
		
			// Clear data type
			dataType &= ~(1 << 5);
		
			// Clear parameter value
			parameterValue[5].clear();
		break;
		
		case 'E':
		
			// Clear data type
			dataType &= ~(1 << 6);
		
			// Clear parameter value
			parameterValue[6].clear();
		break;
		
		case 'F':
		
			// Clear data type
			dataType &= ~(1 << 8);
		
			// Clear parameter value
			parameterValue[7].clear();
		break;
		
		case 'T':
		
			// Clear data type
			dataType &= ~(1 << 9);
		
			// Clear parameter value
			parameterValue[8].clear();
		break;
		
		case 'S':
		
			// Clear data type
			dataType &= ~(1 << 10);
		
			// Clear parameter value
			parameterValue[9].clear();
		break;
		
		case 'P':
		
			// Clear data type
			dataType &= ~(1 << 11);
		
			// Clear parameter value
			parameterValue[10].clear();
		break;
		
		case 'I':
		
			// Clear data type
			dataType &= ~(1 << 16);
		
			// Clear parameter value
			parameterValue[11].clear();
		break;
		
		case 'J':
		
			// Clear data type
			dataType &= ~(1 << 17);
		
			// Clear parameter value
			parameterValue[12].clear();
		break;
		
		case 'R':
		
			// Clear data type
			dataType &= ~(1 << 18);
		
			// Clear parameter value
			parameterValue[13].clear();
		break;
		
		case 'D':
		
			// Clear data type
			dataType &= ~(1 << 19);
		
			// Clear parameter value
			parameterValue[14].clear();
		break;
	}
}

bool Gcode::hasValue(char identifier) const {

	// Check what parameter is requested
	switch(identifier) {
		
		case 'N':
		
			// Return if parameter's value isn't empty
			return !parameterValue[0].empty();
		break;
		
		case 'M':
		
			// Return if parameter's value isn't empty
			return !parameterValue[1].empty();
		break;
		
		case 'G':
		
			// Return if parameter's value isn't empty
			return !parameterValue[2].empty();
		break;
		
		case 'X':
		
			// Return if parameter's value isn't empty
			return !parameterValue[3].empty();
		break;
		
		case 'Y':
		
			// Return if parameter's value isn't empty
			return !parameterValue[4].empty();
		break;
		
		case 'Z':
		
			// Return if parameter's value isn't empty
			return !parameterValue[5].empty();
		break;
		
		case 'E':
		
			// Return if parameter's value isn't empty
			return !parameterValue[6].empty();
		break;
		
		case 'F':
		
			// Return if parameter's value isn't empty
			return !parameterValue[7].empty();
		break;
		
		case 'T':
		
			// Return if parameter's value isn't empty
			return !parameterValue[8].empty();
		break;
		
		case 'S':
		
			// Return if parameter's value isn't empty
			return !parameterValue[9].empty();
		break;
		
		case 'P':
		
			// Return if parameter's value isn't empty
			return !parameterValue[10].empty();
		break;
		
		case 'I':
		
			// Return if parameter's value isn't empty
			return !parameterValue[11].empty();
		break;
		
		case 'J':
		
			// Return if parameter's value isn't empty
			return !parameterValue[12].empty();
		break;
		
		case 'R':
		
			// Return if parameter's value isn't empty
			return !parameterValue[13].empty();
		break;
		
		case 'D':
		
			// Return if parameter's value isn't empty
			return !parameterValue[15].empty();
		break;
	}
	
	// Return false
	return false;
}

string Gcode::getValue(char identifier) const {

	// Check what parameter is requested
	switch(identifier) {
		
		case 'N':
		
			// Return value
			return parameterValue[0];
		break;
		
		case 'M':
		
			// Return value
			return parameterValue[1];
		break;
		
		case 'G':
		
			// Return value
			return parameterValue[2];
		break;
		
		case 'X':
		
			// Return value
			return parameterValue[3];
		break;
		
		case 'Y':
		
			// Return value
			return parameterValue[4];
		break;
		
		case 'Z':
		
			// Return value
			return parameterValue[5];
		break;
		
		case 'E':
		
			// Return value
			return parameterValue[6];
		break;
		
		case 'F':
		
			// Return value
			return parameterValue[7];
		break;
		
		case 'T':
		
			// Return value
			return parameterValue[8];
		break;
		
		case 'S':
		
			// Return value
			return parameterValue[9];
		break;
		
		case 'P':
		
			// Return value
			return parameterValue[10];
		break;
		
		case 'I':
		
			// Return value
			return parameterValue[11];
		break;
		
		case 'J':
		
			// Return value
			return parameterValue[12];
		break;
		
		case 'R':
		
			// Return value
			return parameterValue[13];
		break;
		
		case 'D':
		
			// Return value
			return parameterValue[14];
		break;
	}
	
	// Return empty
	return "";
}

void Gcode::setValue(char identifier, const string &value) {

	// Clear empty
	empty = false;
	
	// Set parsed
	parsed = true;

	// Check what parameter is set
	switch(identifier) {
		
		case 'N':
		
			// Set data type
			dataType |= 1;
		
			// Set parameter value
			parameterValue[0] = value;
		break;
		
		case 'M':
		
			// Set data type
			dataType |= (1 << 1);
		
			// Set parameter value
			parameterValue[1] = value;
		break;
		
		case 'G':
		
			// Set data type
			dataType |= (1 << 2);
		
			// Set parameter value
			parameterValue[2] = value;
		break;
		
		case 'X':
		
			// Set data type
			dataType |= (1 << 3);
		
			// Set parameter value
			parameterValue[3] = value;
		break;
		
		case 'Y':
		
			// Set data type
			dataType |= (1 << 4);
		
			// Set parameter value
			parameterValue[4] = value;
		break;
		
		case 'Z':
		
			// Set data type
			dataType |= (1 << 5);
		
			// Set parameter value
			parameterValue[5] = value;
		break;
		
		case 'E':
		
			// Set data type
			dataType |= (1 << 6);
		
			// Set parameter value
			parameterValue[6] = value;
		break;
		
		case 'F':
		
			// Set data type
			dataType |= (1 << 8);
		
			// Set parameter value
			parameterValue[7] = value;
		break;
		
		case 'T':
		
			// Set data type
			dataType |= (1 << 9);
		
			// Set parameter value
			parameterValue[8] = value;
		break;
		
		case 'S':
		
			// Set data type
			dataType |= (1 << 10);
		
			// Set parameter value
			parameterValue[9] = value;
		break;
		
		case 'P':
		
			// Set data type
			dataType |= (1 << 11);
		
			// Set parameter value
			parameterValue[10] = value;
		break;
		
		case 'I':
		
			// Set data type
			dataType |= (1 << 16);
		
			// Set parameter value
			parameterValue[11] = value;
		break;
		
		case 'J':
		
			// Set data type
			dataType |= (1 << 17);
		
			// Set parameter value
			parameterValue[12] = value;
		break;
		
		case 'R':
		
			// Set data type
			dataType |= (1 << 18);
		
			// Set parameter value
			parameterValue[13] = value;
		break;
		
		case 'D':
		
			// Set data type
			dataType |= (1 << 19);
		
			// Set parameter value
			parameterValue[14] = value;
		break;
	}
}

bool Gcode::hasString() const {

	// Return if string is set
	return dataType & (1 << 15);
}

string Gcode::getString() const {

	// Return string
	return parameterValue[15];
}

void Gcode::setString(const string &value) {

	// Clear empty
	empty = false;
	
	// Set parsed
	parsed = true;

	// Set data type
	dataType |= (1 << 15);

	// Set string value
	parameterValue[15] = value;
}

void Gcode::clear() {

	// Set parameter value size
	parameterValue.clear();
	parameterValue.resize(16);
	
	// Set inital data type
	dataType = 0x1080;
	
	// Clear parsed
	parsed = false;
	
	// Set empty
	empty = true;
	
	// Clear host command
	hostCommand.clear();
	
	// Clear original command
	originalCommand.clear();
}

bool Gcode::isParsed() const {

	// Return parsed
	return parsed;
}

bool Gcode::isHostCommand() const {

	// Return if host command is set
	return !hostCommand.empty();
}

bool Gcode::isEmpty() const {

	// Return if empty is set
	return empty;
}

Gcode &Gcode::operator=(const Gcode &value) {

	// Return self if calling on self
	if(this == &value)
		return *this;
	
	// Copy data type
	dataType = value.dataType;
	
	// Copy host command
	hostCommand = value.hostCommand;
	
	// Copy parameter values
	parameterValue = value.parameterValue;
	
	// Copy parsed
	parsed = value.parsed;
	
	// Copy original command
	originalCommand = value.originalCommand;
	
	// Copy empty
	empty = value.empty;
	
	// Return self
	return *this;
}

ostream &operator<<(ostream &output, const Gcode &gcode) {

	// Return command sent to output
	return output << (gcode.parsed ? gcode.getAscii() : gcode.originalCommand);
}
