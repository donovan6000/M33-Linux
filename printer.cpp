// Header files
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <cmath>
#include <cfloat>
#include <queue>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <pwd.h>
#include "printer.h"


// Definitions

// EEPROM offsets
#define EEPROM_BACKLASH_X 0
#define EEPROM_BACKLASH_Y 1
#define EEPROM_BACK_RIGHT_ORIENTATION 2
#define EEPROM_BACK_LEFT_ORIENTATION 3
#define EEPROM_FRONT_LEFT_ORIENTATION 4
#define EEPROM_FRONT_RIGHT_ORIENTATION 5
#define EEPROM_FILAMENT_COLOR 6
#define EEPROM_FILAMENT_TYPE 7
#define EEPROM_FILAMENT_TEMPERATURE 8
#define EEPROM_FILAMENT_AMOUNT 9
#define EEPROM_BACKLASH_EXPANSION_X_PLUS 10
#define EEPROM_BACKLASH_EXPANSION_YL_PLUS 11
#define EEPROM_BACKLASH_EXPANSION_YR_PLUS 12
#define EEPROM_BACKLASH_EXPANSION_YR_MINUS 13
#define EEPROM_BACKLASH_EXPANSION_Z 14
#define EEPROM_BACKLASH_EXPANSION_E 15
#define EEPROM_BACK_LEFT_OFFSET 16
#define EEPROM_BACK_RIGHT_OFFSET 17
#define EEPROM_FRONT_RIGHT_OFFSET 18
#define EEPROM_FRONT_LEFT_OFFSET 19
#define EEPROM_BACKLASH_SPEED 22
#define EEPROM_BED_HEIGHT_OFFSET 32

// Chip details
#define CHIP_NAME ATxmega32C4
#define CHIP_PAGE_SIZE 0x80
#define CHIP_NRWW_SIZE 0x20
#define CHIP_NUMBER_OF_PAGES 0x80
#define CHIP_TOTAL_MEMORY CHIP_NUMBER_OF_PAGES * CHIP_PAGE_SIZE * 2

// Wave bonding settings
#define WAVE_PERIOD 5.0
#define WAVE_PERIOD_QUARTER (WAVE_PERIOD / 4.0)
#define WAVE_SIZE 0.15

// Bed compensation settings
#define LEVELLING_MOVE_X 104.9
#define LEVELLING_MOVE_Y 103.0
#define SEGMENT_LENGTH 2.0

// Feed rate conversion settings
#define MAX_FEED_RATE 60.0001

// Fan types
enum fanTypes {HENGLIXIN = 0x01, LISTENER = 0x02, SHENZHEW = 0x03, NO_FAN = 0xFF};

// Directions
enum directions {POSITIVE, NEGATIVE, NEITHER};

// Print tiers
enum printTiers {LOW, MEDIUM, HIGH};


// Rom decryption and encryption tables
const uint8_t romDecryptionTable[] = {0x26, 0xE2, 0x63, 0xAC, 0x27, 0xDE, 0x0D, 0x94, 0x79, 0xAB, 0x29, 0x87, 0x14, 0x95, 0x1F, 0xAE, 0x5F, 0xED, 0x47, 0xCE, 0x60, 0xBC, 0x11, 0xC3, 0x42, 0xE3, 0x03, 0x8E, 0x6D, 0x9D, 0x6E, 0xF2, 0x4D, 0x84, 0x25, 0xFF, 0x40, 0xC0, 0x44, 0xFD, 0x0F, 0x9B, 0x67, 0x90, 0x16, 0xB4, 0x07, 0x80, 0x39, 0xFB, 0x1D, 0xF9, 0x5A, 0xCA, 0x57, 0xA9, 0x5E, 0xEF, 0x6B, 0xB6, 0x2F, 0x83, 0x65, 0x8A, 0x13, 0xF5, 0x3C, 0xDC, 0x37, 0xD3, 0x0A, 0xF4, 0x77, 0xF3, 0x20, 0xE8, 0x73, 0xDB, 0x7B, 0xBB, 0x0B, 0xFA, 0x64, 0x8F, 0x08, 0xA3, 0x7D, 0xEB, 0x5C, 0x9C, 0x3E, 0x8C, 0x30, 0xB0, 0x7F, 0xBE, 0x2A, 0xD0, 0x68, 0xA2, 0x22, 0xF7, 0x1C, 0xC2, 0x17, 0xCD, 0x78, 0xC7, 0x21, 0x9E, 0x70, 0x99, 0x1A, 0xF8, 0x58, 0xEA, 0x36, 0xB1, 0x69, 0xC9, 0x04, 0xEE, 0x3B, 0xD6, 0x34, 0xFE, 0x55, 0xE7, 0x1B, 0xA6, 0x4A, 0x9A, 0x54, 0xE6, 0x51, 0xA0, 0x4E, 0xCF, 0x32, 0x88, 0x48, 0xA4, 0x33, 0xA5, 0x5B, 0xB9, 0x62, 0xD4, 0x6F, 0x98, 0x6C, 0xE1, 0x53, 0xCB, 0x46, 0xDD, 0x01, 0xE5, 0x7A, 0x86, 0x75, 0xDF, 0x31, 0xD2, 0x02, 0x97, 0x66, 0xE4, 0x38, 0xEC, 0x12, 0xB7, 0x00, 0x93, 0x15, 0x8B, 0x6A, 0xC5, 0x71, 0x92, 0x45, 0xA1, 0x59, 0xF0, 0x06, 0xA8, 0x5D, 0x82, 0x2C, 0xC4, 0x43, 0xCC, 0x2D, 0xD5, 0x35, 0xD7, 0x3D, 0xB2, 0x74, 0xB3, 0x09, 0xC6, 0x7C, 0xBF, 0x2E, 0xB8, 0x28, 0x9F, 0x41, 0xBA, 0x10, 0xAF, 0x0C, 0xFC, 0x23, 0xD9, 0x49, 0xF6, 0x7E, 0x8D, 0x18, 0x96, 0x56, 0xD1, 0x2B, 0xAD, 0x4B, 0xC1, 0x4F, 0xC8, 0x3A, 0xF1, 0x1E, 0xBD, 0x4C, 0xDA, 0x50, 0xA7, 0x52, 0xE9, 0x76, 0xD8, 0x19, 0x91, 0x72, 0x85, 0x3F, 0x81, 0x61, 0xAA, 0x05, 0x89, 0x0E, 0xB5, 0x24, 0xE0};

const uint8_t romEncryptionTable[] = {0xAC, 0x9C, 0xA4, 0x1A, 0x78, 0xFA, 0xB8, 0x2E, 0x54, 0xC8, 0x46, 0x50, 0xD4, 0x06, 0xFC, 0x28, 0xD2, 0x16, 0xAA, 0x40, 0x0C, 0xAE, 0x2C, 0x68, 0xDC, 0xF2, 0x70, 0x80, 0x66, 0x32, 0xE8, 0x0E, 0x4A, 0x6C, 0x64, 0xD6, 0xFE, 0x22, 0x00, 0x04, 0xCE, 0x0A, 0x60, 0xE0, 0xBC, 0xC0, 0xCC, 0x3C, 0x5C, 0xA2, 0x8A, 0x8E, 0x7C, 0xC2, 0x74, 0x44, 0xA8, 0x30, 0xE6, 0x7A, 0x42, 0xC4, 0x5A, 0xF6, 0x24, 0xD0, 0x18, 0xBE, 0x26, 0xB4, 0x9A, 0x12, 0x8C, 0xD8, 0x82, 0xE2, 0xEA, 0x20, 0x88, 0xE4, 0xEC, 0x86, 0xEE, 0x98, 0x84, 0x7E, 0xDE, 0x36, 0x72, 0xB6, 0x34, 0x90, 0x58, 0xBA, 0x38, 0x10, 0x14, 0xF8, 0x92, 0x02, 0x52, 0x3E, 0xA6, 0x2A, 0x62, 0x76, 0xB0, 0x3A, 0x96, 0x1C, 0x1E, 0x94, 0x6E, 0xB2, 0xF4, 0x4C, 0xC6, 0xA0, 0xF0, 0x48, 0x6A, 0x08, 0x9E, 0x4E, 0xCA, 0x56, 0xDA, 0x5E, 0x2F, 0xF7, 0xBB, 0x3D, 0x21, 0xF5, 0x9F, 0x0B, 0x8B, 0xFB, 0x3F, 0xAF, 0x5B, 0xDB, 0x1B, 0x53, 0x2B, 0xF3, 0xB3, 0xAD, 0x07, 0x0D, 0xDD, 0xA5, 0x95, 0x6F, 0x83, 0x29, 0x59, 0x1D, 0x6D, 0xCF, 0x87, 0xB5, 0x63, 0x55, 0x8D, 0x8F, 0x81, 0xED, 0xB9, 0x37, 0xF9, 0x09, 0x03, 0xE1, 0x0F, 0xD3, 0x5D, 0x75, 0xC5, 0xC7, 0x2D, 0xFD, 0x3B, 0xAB, 0xCD, 0x91, 0xD1, 0x4F, 0x15, 0xE9, 0x5F, 0xCB, 0x25, 0xE3, 0x67, 0x17, 0xBD, 0xB1, 0xC9, 0x6B, 0xE5, 0x77, 0x35, 0x99, 0xBF, 0x69, 0x13, 0x89, 0x61, 0xDF, 0xA3, 0x45, 0x93, 0xC1, 0x7B, 0xC3, 0xF1, 0xD7, 0xEB, 0x4D, 0x43, 0x9B, 0x05, 0xA1, 0xFF, 0x97, 0x01, 0x19, 0xA7, 0x9D, 0x85, 0x7F, 0x4B, 0xEF, 0x73, 0x57, 0xA9, 0x11, 0x79, 0x39, 0xB7, 0xE7, 0x1F, 0x49, 0x47, 0x41, 0xD9, 0x65, 0x71, 0x33, 0x51, 0x31, 0xD5, 0x27, 0x7D, 0x23};

// Crc seed and table
const uint32_t crc32Seed = 0xFFFFFFFF;

const uint32_t crc32Table[] = {0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1, 0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D};


// Supporting function implementation
Printer::Printer() {

	// Initialize variables
	char* tempPath;

	// Clear file descriptor and virtual port
	fd = -1;
	vd = -1;
	
	// Clear status
	status = 0;
	
	// Set bootloader mode
	bootloaderMode = true;
	
	// Get temp directory
	tempPath = getenv("TEMP");
	if(tempPath == NULL)
		 tempPath = getenv("TMP");
	if(tempPath == NULL)
		 tempPath = getenv("TMPDIR");
	
	// Create temporary folder
	workingFolderLocation = mkdtemp(const_cast<char *>((static_cast<string>(tempPath == NULL ? P_tmpdir : tempPath) + "/m33-XXXXXX").c_str()));
	
	// Clear all use pre-processor stages
	useValidation = false;
	usePreparation = false;
	useWaveBonding = false;
	useThermalBonding = false;
	useBedCompensation = false;
	useBacklashCompensation = false;
	useFeedRateConversion = false;
	
	// Set values to their defaults
	backlashX = 0.3;
	backlashY = 0.6;
	backlashSpeed = 1500;
	backRightOffset = 0;
	backLeftOffset = 0;
	frontLeftOffset = 0;
	frontRightOffset = 0;
	filamentTemperature = 200;
	filamentType = PLA;
}

Printer::~Printer() {

	// Close file descriptor if open
	if(fd != -1)
		close(fd);
	
	// Delete temporary folder
	rmdir(workingFolderLocation.c_str());
	
	// Close virtual port descriptor if open
	if(vd != -1)
		close(vd);
	
	// Delete symbolic link for virtual serial port location
	if(!virtualSerialPortLocation.empty())
		unlink(virtualSerialPortLocation.c_str());
}

bool Printer::connect() {

	// Initialize variables
        termios settings;
        flock lock;
        
        // Close file descriptor if open
        if(fd != -1)
        	close(fd);
        
        // Attempt to connect for 2 seconds
        for(uint8_t i = 0; i < 8; i++) {
        
		// Wait 250 milliseconds
		usleep(250000);
		
		// Check if opening device was successful
		if((fd = open("/dev/micro_3d", O_RDWR | O_NONBLOCK)) != -1) {
		
			// Create file lock
			lock.l_type = F_WRLCK;
			lock.l_start = 0;
			lock.l_whence = SEEK_SET;
			lock.l_len = 0;
			
			// Check if file is already locked by another process
			if(fcntl(fd, F_SETLK, &lock) == -1)
				return false;
	      
			// Set serial protocol to 8n1 with 115200 baud rate
			memset(&settings, 0, sizeof(settings));
			settings.c_iflag = 0;
			settings.c_oflag = 0;
			settings.c_cflag= CS8 | CREAD | CLOCAL;
			settings.c_lflag = 0;
			settings.c_cc[VMIN] = 1;
			settings.c_cc[VTIME] = 5;
			cfsetospeed(&settings, B115200);
			cfsetispeed(&settings, B115200);

			// Apply settings
			tcsetattr(fd, TCSAFLUSH, &settings);
			tcdrain(fd);

			// Return true
			return true;
		}
	}
	
	// Return false
	return false;
}

bool Printer::isBootloaderMode() {

	// Check if printer is connected and receiving commands
	if(fd != -1 && sendRequestAscii("M115"))
	
		// Return if in bootloader mode
		return bootloaderMode = (receiveResponseAscii()[0] == 'B');
	
	// Return false
	return false;
}

string Printer::getFirmwareVersion() {

	// Return firmware version
	return firmwareVersion;
}

bool Printer::isFirmwareValid() {

	// Initialize variables
	string response, eepromSerial;
	uint32_t chipCrc = 0, eepromCrc = 0, eepromFirmware = 0;
	fanTypes eepromFan;
	uint8_t fanOffset;
	float fanScale;
	int32_t *tempPointer;

	// Check if printer is connected and receiving commands
	if(fd != -1 && sendRequestAscii("M115")) {
	
		// Check if in bootloader mode
		if(receiveResponseAscii()[0] == 'B') {
	
			// Request crc from chip
			sendRequestAscii('C');
			sendRequestAscii('A');
		
			// Get response
			response = receiveResponseAscii();
		
			// Get chip crc
			for(uint8_t i = 0; i < 4; i++) {
				chipCrc <<= 8;
				chipCrc += static_cast<uint8_t>(response[i]);
			}
		
			// Request eeprom
			sendRequestAscii('S');
		
			// Get response
			response = receiveResponseAscii();
		
			// Check if failed to read eeprom
			if(response.back() != '\r')
		
				// Return false
				return false;
		
			// Get eeprom crc
			for(uint8_t i = 0; i < 4; i++) {
				eepromCrc <<= 8;
				eepromCrc += static_cast<uint8_t>(response[i + 4]);
			}
		
			// Check if firmware is corrupt
			if(chipCrc != eepromCrc)
		
				// Return false
				return false;
			
			// Get eeprom firmware
			for(int8_t i = 3; i >= 0; i--) {
				eepromFirmware <<= 8;
				eepromFirmware += static_cast<uint8_t>(response[i]);
			}
			
			// Check if firmware is deprecated
			if(eepromFirmware < 150994944)
			
				// Return false
				return false;
			
			// Set firmware version
			firmwareVersion = to_string(eepromFirmware);
		
			// Get eeprom serial
			for(uint8_t i = 0; i < 13; i++)
				eepromSerial.push_back(response[i + 0x2EF]);
			
			// Get eeprom fan
			eepromFan = static_cast<fanTypes>(response[0x2AB]);
			
			// Check if fan needs updating
			if(!eepromFan || eepromFan == NO_FAN) {
		
				// Set fan to default
				eepromFan = HENGLIXIN;
			
				// Check if device is newer
				if(stoi(eepromSerial.substr(2, 6)) >= 150602)
			
					// Set fan to newer
					eepromFan = SHENZHEW;
			
				// Set fan offset and scale
				if(eepromFan == HENGLIXIN) {
					fanOffset = 200;
					fanScale = 0.2165354;
				}
				else if(eepromFan == LISTENER) {
					fanOffset = 145;
					fanScale = 0.3333333;
				}
				else {
					fanOffset = 82;
					fanScale = 0.3843137;
				}
				tempPointer = reinterpret_cast<int32_t *>(&fanScale);
			
				// Check if saving fan scale to eeprom failed
				for(uint8_t i = 0; i < 4; i++)
					if(!writeToEeprom(0x2AD + i, *tempPointer >> 8 * i))
				
						// Return false
						return false;
				
				// Check if saving fan offset or fan type to eeprom failed
				if(!writeToEeprom(0x2AC, fanOffset) || !writeToEeprom(0x2AB, eepromFan))
			
					// Return false
					return false;
			}
		
			// Check if extruder current needs updating
			if((eepromSerial == "BK15033001100" || eepromSerial == "BK15040201050" || eepromSerial == "BK15040301050" || eepromSerial == "BK15040602050" || eepromSerial == "BK15040801050" || eepromSerial == "BK15040802100" || eepromSerial == "GR15032702100" || eepromSerial == "GR15033101100" || eepromSerial == "GR15040601100" || eepromSerial == "GR15040701100" || eepromSerial == "OR15032701100" || eepromSerial == "SL15032601050") && static_cast<uint8_t>(response[0x2E8]) + (static_cast<uint8_t>(response[0x2E9]) << 8) != 500)
		
				// Check if saving extruder current to eeprom to be 500 failed
				if(!writeToEeprom(0x2E8, static_cast<uint8_t>(500)) || !writeToEeprom(0x2E9, 500 >> 8))
			
					// Return false
					return false;
		}
		
		// Return true
		return true;
	}
	
	// Return false
	return false;
}

bool Printer::updateFirmware(const char *file) {

	// Initialize variables
	string response, romBuffer, temp;
	uint32_t chipCrc = 0, eepromCrc = 0, position, romVersion = 0, romCrc = 0;
	ifstream romInput;
	uint16_t pagesToWrite;
	uint8_t decryptedRom[CHIP_TOTAL_MEMORY];

	// Check if printer is connected and receiving commands
	if(fd != -1 && sendRequestAscii("M115")) {
	
		// Check if in bootloader mode
		if(receiveResponseAscii()[0] == 'B') {
		
			// Go through the file name
			uint8_t i = 0;
			if(strchr(file, ' ') != NULL)
				i = strchr(file, ' ') - file + 1;
			for(; i < strlen(file); i++) {
			
				// Check if extension is occuring
				if(file[i] == '.') {
				
					// Break if file name beings with 10 numbers
					if(strchr(file, ' ') != NULL && i - (strchr(file, ' ') - file) - 1 == 10)
						break;
					
					if(i == 10)
						break;
					
					// Return false
					return false;
				}
				
				// Check if current character isn't a digit or length is invalid
				if(file[i] < '0' || file[i] > '9' || (i == strlen(file) - 1 && i < 9))
				
					// Return false
					return false;
			}
	
			// Check if opening rom failed
			romInput.open(file, ios::in | ios::binary);
			if(!romInput.good())

				// Return false
				return false;
			
			// Set rom version
			if(strchr(file, ' ') != NULL)
				romVersion = atoi(strchr(file, ' ') + 1);
			else
				romVersion = atoi(file);
			
			// Read in the encrypted rom
			while(romInput.peek() != EOF)
				romBuffer.push_back(romInput.get());
			romInput.close();
			
			// Check if rom isn't encrypted
			if(static_cast<uint8_t>(romBuffer[0]) == 0x0C || static_cast<uint8_t>(romBuffer[0]) == 0xFD) {
			
				// Encrypt the rom
				for(uint16_t i = 0; i < romBuffer.length(); i++)
		
					// Check if padding wasn't required
					if(i % 2 != 0 || i != romBuffer.length() - 1)
			
						// Encrypt the rom
						temp.push_back(romEncryptionTable[static_cast<uint8_t>(romBuffer[i + (i % 2 ? -1 : 1)])]);			
				
				// Set encrypted rom
				romBuffer = temp;
			}
			
			// Check if rom is too big
			if(romBuffer.length() > CHIP_TOTAL_MEMORY)
			
				// Return false
				return false;

			// Request that chip be erased
			sendRequestAscii('E');

			// Check if chip failed to be erased
			do {
				response = receiveResponseAscii();
			} while(response.empty());
			if(response != "\r")

				// Return false
				return false;

			// Send address zero
			sendRequestAscii('A');
			sendRequestAscii('\x00');
			sendRequestAscii('\x00');

			// Check if address wasn't acknowledged
			if(receiveResponseAscii() != "\r")

				// Return false
				return false;

			// Set pages to write
			pagesToWrite = romBuffer.length() / 2 / CHIP_PAGE_SIZE;
			if(romBuffer.length() / 2 % CHIP_PAGE_SIZE != 0)
				pagesToWrite++;

			// Go through all pages to write
			for(uint16_t i = 0; i < pagesToWrite; i++) {

				// Send write to page request
				sendRequestAscii('B');
				sendRequestAscii(CHIP_PAGE_SIZE * 2 >> 8);
				sendRequestAscii(static_cast<char>(CHIP_PAGE_SIZE * 2));

				// Go through all values for the page
				for(int j = 0; j < CHIP_PAGE_SIZE * 2; j++) {

					// Check if data to be written exists
					position = j + CHIP_PAGE_SIZE * i * 2;
					if(position < romBuffer.length())
	
						// Send value
						sendRequestAscii(romBuffer[position + (position % 2 ? -1 : 1)]);
	
					// Otherwise
					else
	
						// Send padding
						sendRequestAscii(romEncryptionTable[0xFF]);
				}

				// Check if chip failed to be flashed
				if(receiveResponseAscii() != "\r")

					// Return false
					return false;
			}

			// Send address zero
			sendRequestAscii('A');
			sendRequestAscii('\x00');
			sendRequestAscii('\x00');

			// Check if address wasn't acknowledged
			if(receiveResponseAscii() != "\r")

				// Return false
				return false;
			
			// Request eeprom
			sendRequestAscii('S');
		
			// Get response
			response = receiveResponseAscii();
		
			// Check if failed to read eeprom
			if(response.back() != '\r')
		
				// Return false
				return false;

			// Check if section needs to be zeroed out
			if(!response[0x2E6] || chipCrc != eepromCrc)

				// Check if zeroing out section failed
				if(!writeToEeprom(0x08, 0) || !writeToEeprom(0x09, 0) || !writeToEeprom(0x0A, 0) || !writeToEeprom(0x0B, 0))

					// Return false
					return false;

			// Request crc from chip
			sendRequestAscii('C');
			sendRequestAscii('A');

			// Get response
			temp = receiveResponseAscii();

			// Get chip crc
			for(uint8_t i = 0; i < 4; i++) {
				chipCrc <<= 8;
				chipCrc += static_cast<uint8_t>(temp[i]);
			}
			
			// Decrypt the rom
			for(uint16_t i = 0; i < CHIP_TOTAL_MEMORY; i++) {
			
				// Check if data exists in the rom
				if (i < romBuffer.length()) {
				
					// Check if padding is required
					if(i % 2 == 0 && i == romBuffer.length() - 1)
					
						// Put padding
						decryptedRom[i] = 0xFF;
						
					// Otherwise
					else
					
						// Decrypt the rom
						decryptedRom[i] = romDecryptionTable[static_cast<uint8_t>(romBuffer[i + (i % 2 ? -1 : 1)])];
				}
				
				// Otherwise
				else
				
					// Put padding
					decryptedRom[i] = 0xFF;
			}
			
			// Get rom crc
			romCrc =  crc32(0, decryptedRom, CHIP_TOTAL_MEMORY);
			
			// Check if firmware update failed
			if(chipCrc != __builtin_bswap32(romCrc))

				// Return false
				return false;

			// Check if zeroing out sections in eeprom failed
			if(!writeToEeprom(0x2D6, 0) || !writeToEeprom(0x2D7, 0) || !writeToEeprom(0x2D8, 0) || !writeToEeprom(0x2D9, 0) || !writeToEeprom(0x2DA, 0) || !writeToEeprom(0x2DB, 0) || !writeToEeprom(0x2DC, 0) || !writeToEeprom(0x2DD, 0) || !writeToEeprom(0x2DE, 0) || !writeToEeprom(0x2DF, 0) || !writeToEeprom(0x2E0, 0) || !writeToEeprom(0x2E1, 0) || !writeToEeprom(0x2E2, 0) || !writeToEeprom(0x2E3, 0) || !writeToEeprom(0x2E4, 0) || !writeToEeprom(0x2E5, 0))

				// Return false
				return false;

			// Check if updating firmware version in eeprom failed
			for(uint8_t i = 0; i < 4; i++)
				if(!writeToEeprom(i, romVersion >> 8 * i))

					// Return false
					return false;

			// Check if updating firmware crc in eeprom failed
			for(uint8_t i = 0; i < 4; i++)
				if(!writeToEeprom(i + 4, romCrc >> 8 * i))

					// Return false
					return false;
			
			// Set firmware version
			firmwareVersion = to_string(romVersion);
		}
		
		// Return true
		return true;
	}
	
	// Return false
	return false;
}

bool Printer::isZValid() {

	// Retrurn valid Z
	return validZ;
}
		
void Printer::calibrateZ() {

	// Initialize variables
	string response;

	// Turn off heater
	do {
		sendRequest("M104 S0");
		do {
			response = receiveResponse();
			while(response.substr(0, 2) == "T:")
				response = receiveResponse();
		} while(response.empty());
	} while(response.substr(0, 2) != "ok");
	
	// Delay
	do {
		sendRequest("G4 S10");
		do {
			response = receiveResponse();
		} while(response.empty());
	} while(response.substr(0, 2) != "ok");
	
	// Relative mode
	do {
		sendRequest("G91");
		do {
			response = receiveResponse();
		} while(response.empty());
	} while(response.substr(0, 2) != "ok");
	
	// Move to position
	do {
		sendRequest("G0 Y20 Z2 F150");
		do {
			response = receiveResponse();
		} while(response.empty());
	} while(response.substr(0, 2) != "ok");
	
	// Set heater temperature and wait
	do {
		sendRequest("M109 S150");
		do {
			response = receiveResponse();
		} while(response.empty());
	} while(response.substr(0, 2) != "ok");
	
	// Turn off heater
	do {
		sendRequest("M104 S0");
		do {
			response = receiveResponse();
			while(response.substr(0, 2) == "T:")
				response = receiveResponse();
		} while(response.empty());
	} while(response.substr(0, 2) != "ok");
	
	// Turn off fan
	do {
		sendRequest("M106 S0");
		do {
			response = receiveResponse();
		} while(response.empty());
	} while(response.substr(0, 2) != "ok");
	
	// Calibrate Z
	do {
		sendRequest("G30");
		do {
			response = receiveResponse();
		} while(response.empty());
	} while(response.substr(0, 2) != "ok");
	
	// Verify results
	do {
		sendRequest("M577 F0");
		do {
			response = receiveResponse();
		} while(response.empty());
	} while(response.substr(0, 2) != "ok");
	
	// Set valid Z
	validZ = true;
}

bool Printer::isBedOrientationValid() {

	// Retrurn valid bed orientation
	return validBedOrientation;
}

void Printer::calibrateBedOrientation() {
	
	// Set valid bed orientation
	validBedOrientation = true;
}
		
bool Printer::collectInformation() {

	// Initialize variables
	string response;
	char character;
	uint32_t value;
	float *valuePointer;

	// Loop forever
	while(1) {
	
		// Check if device is already in g-code processing mode
		sendRequestAscii("M115");
		while(read(fd, &character, 1) == -1);
		if(character == 'e')
		
			// Break
			break;
		
		// Attempt to put device into g-code processing mode
		sendRequestAscii('Q');
		
		// Return false if failed to reconnect
		if(!connect())
			return false;
	}
	
	// Clear bootloader
	bootloaderMode = false;
	
	// Catch string errors
	try {
	
		// Get device info
		sendRequest("M115");
		response = receiveResponse();
		
		// Set firmware and serial number 
		firmwareVersion = response.substr(response.find("FIRMWARE_VERSION:") + 17, response.find(" ", response.find("FIRMWARE_VERSION:")) - response.find("FIRMWARE_VERSION:") - 17);
		serialNumber = response.substr(response.find("SERIAL_NUMBER:") + 14);
		
		// Display values
		cout << "Firmware Version: " << firmwareVersion << endl;
		cout << "Serial Number: " << serialNumber << endl;
		
		// Get back right offset
		sendRequest("M619 S" + to_string(EEPROM_BACK_RIGHT_OFFSET));
		response = receiveResponse();
		value = stoi(response.substr(response.find("DT:") + 3));
		valuePointer = reinterpret_cast<float *>(&value);
		backRightOffset = *valuePointer;
		
		// Get back left offset
		sendRequest("M619 S" + to_string(EEPROM_BACK_LEFT_OFFSET));
		response = receiveResponse();
		value = stoi(response.substr(response.find("DT:") + 3));
		valuePointer = reinterpret_cast<float *>(&value);
		backLeftOffset = *valuePointer;
		
		// Get front left offset
		sendRequest("M619 S" + to_string(EEPROM_FRONT_LEFT_OFFSET));
		response = receiveResponse();
		value = stoi(response.substr(response.find("DT:") + 3));
		valuePointer = reinterpret_cast<float *>(&value);
		frontLeftOffset = *valuePointer;
		
		// Get front right offset
		sendRequest("M619 S" + to_string(EEPROM_FRONT_RIGHT_OFFSET));
		response = receiveResponse();
		value = stoi(response.substr(response.find("DT:") + 3));
		valuePointer = reinterpret_cast<float *>(&value);
		frontRightOffset = *valuePointer;
		
		// Get bed height offset
		sendRequest("M619 S" + to_string(EEPROM_BED_HEIGHT_OFFSET));
		response = receiveResponse();
		value = stoi(response.substr(response.find("DT:") + 3));
		valuePointer = reinterpret_cast<float *>(&value);
		bedHeightOffset = *valuePointer;
		
		// Display values
		cout << "Back Right Offset: " << backRightOffset << endl;
		cout << "Back Left Offset: " << backLeftOffset << endl;
		cout << "Front Left Offset: " << frontLeftOffset << endl;
		cout << "Front Right Offset: " << frontRightOffset << endl;
		cout << "Bed Height Offset: " << bedHeightOffset << endl;
		
		// Get backlash X
		sendRequest("M619 S" + to_string(EEPROM_BACKLASH_X));
		response = receiveResponse();
		value = stoi(response.substr(response.find("DT:") + 3));
		valuePointer = reinterpret_cast<float *>(&value);
		backlashX = *valuePointer;
		
		// Get backlash Y
		sendRequest("M619 S" + to_string(EEPROM_BACKLASH_Y));
		response = receiveResponse();
		value = stoi(response.substr(response.find("DT:") + 3));
		valuePointer = reinterpret_cast<float *>(&value);
		backlashY = *valuePointer;
		
		// Get backlash speed
		sendRequest("M619 S" + to_string(EEPROM_BACKLASH_SPEED));
		response = receiveResponse();
		value = stoi(response.substr(response.find("DT:") + 3));
		valuePointer = reinterpret_cast<float *>(&value);
		backlashSpeed = *valuePointer;
		
		// Check if backlash speed isn't valid
		if(backlashSpeed <= 1 || backlashSpeed >= 5000) {
		
			// Set backlash speed to default value
			sendRequestBinary("M618 S22 P1153138688");
			receiveResponse();
			backlashSpeed = 1500;
		}
		
		// Display values
		cout << "Backlash X: " << backlashX << endl;
		cout << "Backlash Y: " << backlashY << endl;
		cout << "Backlash Speed: " << backlashSpeed << endl;
		
		// Get back right orientation
		sendRequest("M619 S" + to_string(EEPROM_BACK_RIGHT_ORIENTATION));
		response = receiveResponse();
		value = stoi(response.substr(response.find("DT:") + 3));
		valuePointer = reinterpret_cast<float *>(&value);
		backRightOrientation = *valuePointer;
		
		// Get back left orientation
		sendRequest("M619 S" + to_string(EEPROM_BACK_LEFT_ORIENTATION));
		response = receiveResponse();
		value = stoi(response.substr(response.find("DT:") + 3));
		valuePointer = reinterpret_cast<float *>(&value);
		backLeftOrientation = *valuePointer;
		
		// Get front left orientation
		sendRequest("M619 S" + to_string(EEPROM_FRONT_LEFT_ORIENTATION));
		response = receiveResponse();
		value = stoi(response.substr(response.find("DT:") + 3));
		valuePointer = reinterpret_cast<float *>(&value);
		frontLeftOrientation = *valuePointer;
		
		// Get front right orientation
		sendRequest("M619 S" + to_string(EEPROM_FRONT_RIGHT_ORIENTATION));
		response = receiveResponse();
		value = stoi(response.substr(response.find("DT:") + 3));
		valuePointer = reinterpret_cast<float *>(&value);
		frontRightOrientation = *valuePointer;
		
		// Set valid bed orientation
		validBedOrientation = (backRightOrientation != 0 || backLeftOrientation != 0 || frontLeftOrientation != 0 || frontRightOrientation != 0) && backRightOrientation >= -3 && backRightOrientation <= 3 && backLeftOrientation >= -3 && backLeftOrientation <= 3 && frontLeftOrientation >= -3 && frontLeftOrientation <= 3 && frontRightOrientation >= -3 && frontRightOrientation <= 3;
		
		// Display values
		cout << "Back Right Orientation: " << backRightOrientation << endl;
		cout << "Back Left Orientation: " << backLeftOrientation << endl;
		cout << "Front Left Orientation: " << frontLeftOrientation << endl;
		cout << "Front Right Orientation: " << frontRightOrientation << endl;
		
		// Get status
		sendRequestBinary("M117");
		response = receiveResponse();
	
		// Set valid Z and status
		validZ = response.find("ZV:1") != string::npos; 
		status = stoi(response.substr(response.find("S:") + 2));
		
		cout << "Zalid Z: " << validZ << endl;
		cout << "Status: " << static_cast<unsigned int>(status) << endl;
		
		// Get filament type
		sendRequest("M619 S" + to_string(EEPROM_FILAMENT_TYPE));
		response = receiveResponse();
		value = stoi(response.substr(response.find("DT:") + 3));
		filamentLocation = (value & 0xC0) == 0x00 ? NO_LOCATION : (value & 0xC0) == 0x40 ? INTERNAL : EXTERNAL;
		filamentType = (value & 0x3F) < 4 ? static_cast<filamentTypes>(value & 0x3F) : NO_TYPE;
		
		// Get filament color
		sendRequest("M619 S" + to_string(EEPROM_FILAMENT_COLOR));
		response = receiveResponse();
		value = stoi(response.substr(response.find("DT:") + 3));
		filamentColor = value <= 0x2C ? static_cast<filamentColors>(value) : OTHER_COLOR;
		
		// Get filament temperature
		sendRequest("M619 S" + to_string(EEPROM_FILAMENT_TEMPERATURE));
		response = receiveResponse();
		filamentTemperature = stoi(response.substr(response.find("DT:") + 3)) + 100;
		
		// Display values
		cout << "Filament Location: " << filamentLocation << endl;
		cout << "Filament Type: " << filamentType << endl;
		cout << "Filament Color: " << filamentColor<< endl;
		cout << "Filament Temperature: " << filamentTemperature << endl;
	}
	
	// Check if an out of range error has occured
	catch(const out_of_range& exception) {
	
		// Return false
		return false;
	}
	
	// Check if an invalid argument error has occured
	catch(const invalid_argument& exception) {
	
		// Return false
		return false;
	}
	
	// Check if creating settings file failes
	if(!createSettingsFile()) {
	
		// Display error
		cout << "Could not create settings file" << endl;
		return false;
	}
	
	//return true
	return true;
}
	
bool Printer::sendRequest(const char *data) {

	// Send data based on if in bootloader mode
	return bootloaderMode ? sendRequestAscii(data) : sendRequestBinary(data);
}

bool Printer::sendRequest(const string &data) {

	// Return if data was sent 
	return sendRequest(data.c_str());
}

bool Printer::sendRequest(const Gcode &data) {

	// Send data based on if in bootloader mode
	return bootloaderMode ? sendRequestAscii(data) : sendRequestBinary(data);
}
	
string Printer::receiveResponse() {

	// Send data based on if in bootloader mode
	return bootloaderMode ? receiveResponseAscii() : receiveResponseBinary();
}

bool Printer::processFile(const char *inputFile, const char *outputFile) {

	// Initialize variables
	fstream processedFile;
	ifstream input;
	ofstream output;
	char userName[256];
	passwd *pwd;
	
	// Check if opening input and creating processed file wern't successful
	input.open(inputFile, ios::in | ios::binary);
	processedFile.open(workingFolderLocation + "/output.gcode", ios::out | ios::binary | ios::app);
	if(!input.good() || !processedFile.good())
	
		// Return false
		return false;
	
	// Read in file
	processedFile << input.rdbuf();
	processedFile.close();
	
	// Display message
	cout << "Processing " << inputFile << endl;
	
	// Use center model preprocessor if set
	if(useCenterModel) {
		
		// Check if preprocessor failed
		if(!centerModelPreprocessor((workingFolderLocation + "/output.gcode").c_str())) {
		
			// Display error
			cout << "Center model pre-processor failed" << endl;
			return 0;
		}
		
		// Otherwise
		else
		
			// Display message
			cout << "Center model pre-processor done" << endl;
	}
	
	// Check if print dimensions are out of bounds
	if(!checkPrintDimensions((workingFolderLocation + "/output.gcode").c_str(), false)) {
	
		// Display error
		cout << "Model's dimensions are invalid" << endl;
		return false;
	}
	
	// Use validation preprocessor if set
	if(useValidation) {
		
		// Check if preprocessor failed
		if(!validationPreprocessor((workingFolderLocation + "/output.gcode").c_str())) {
		
			// Display error
			cout << "Validation pre-processor failed" << endl;
			return 0;
		}
		
		// Otherwise
		else
		
			// Display message
			cout << "Validation pre-processor done" << endl;
	}
	
	// Use preparation preprocessor if set
	if(usePreparation) {
		
		// Check if preprocessor failed
		if(!preparationPreprocessor((workingFolderLocation + "/output.gcode").c_str(), false)) {
		
			// Display error
			cout << "Preparation pre-processor failed" << endl;
			return 0;
		}
		
		// Otherwise
		else
		
			// Display message
			cout << "Preparation pre-processor done" << endl;
	}
	
	// Use wave bonding preprocessor if set
	if(useWaveBonding) {
		
		// Check if preprocessor failed
		if(!waveBondingPreprocessor((workingFolderLocation + "/output.gcode").c_str())) {
		
			// Display error
			cout << "Wave bonding pre-processor failed" << endl;
			return 0;
		}
		
		// Otherwise
		else
		
			// Display message
			cout << "Wave bonding pre-processor done" << endl;
	}
	
	// Use thermal bonding preprocessor if set
	if(useThermalBonding) {
		
		// Check if preprocessor failed
		if(!thermalBondingPreprocessor((workingFolderLocation + "/output.gcode").c_str(), false)) {
		
			// Display error
			cout << "Thermal bonding pre-processor failed" << endl;
			return 0;
		}
		
		// Otherwise
		else
		
			// Display message
			cout << "Thermal bonding pre-processor done" << endl;
	}
	
	// Use bed compensation preprocessor if set
	if(useBedCompensation) {
		
		// Check if preprocessor failed
		if(!bedCompensationPreprocessor((workingFolderLocation + "/output.gcode").c_str())) {
		
			// Display error
			cout << "Bed compensation pre-processor failed" << endl;
			return 0;
		}
		
		// Otherwise
		else
		
			// Display message
			cout << "Bed compensation pre-processor done" << endl;
	}
	
	// Use backlash compensation preprocessor if set
	if(useBacklashCompensation) {
		
		// Check if preprocessor failed
		if(!backlashCompensationPreprocessor((workingFolderLocation + "/output.gcode").c_str())) {
		
			// Display error
			cout << "Backlash compensation pre-processor failed" << endl;
			return 0;
		}
		
		// Otherwise
		else
		
			// Display message
			cout << "Backlash compensation pre-processor done" << endl;
	}
	
	// Use feed rate conversion proprocessor if set
	if(useFeedRateConversion) {
		
		// Check if preprocessor failed
		if(!feedRateConversionPreprocessor((workingFolderLocation + "/output.gcode").c_str())) {
		
			// Display error
			cout << "Feed rate conversion pre-processor failed" << endl;
			return 0;
		}
		
		// Otherwise
		else
		
			// Display message
			cout << "Feed rate conversion pre-processor done" << endl;
	}
	
	// Check if an output file is specified
	if(outputFile != NULL) {
	
		// Check if getting user name was successful
		if(getlogin_r(userName, 256))
		
			// Return false
			return false;
		
		// Check if getting group or user id failed
		if((pwd = getpwnam(userName)) == NULL)
		
			// Return false
			return false;
	
		// Check if opening files failed
		processedFile.open(workingFolderLocation + "/output.gcode", ios::in | ios::binary);
		output.open(outputFile, ios::out | ios::binary);
		if(!output.good() || !processedFile.good())
	
			// Return false
			return false;
	
		// Copy processed file to output
		output << processedFile.rdbuf();
		
		// Close files
		output.close();
		processedFile.close();
		
		// Check if changing permission of output failed
		if(chmod(outputFile, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR) == -1) {
		
			// Delete output file
			unlink(outputFile);
			return false;
		}
		
		// Check if changing ownership of output failed
		if(chown(outputFile, pwd->pw_uid, pwd->pw_gid) == -1) {
		
			// Delete output file
			unlink(outputFile);
			return false;
		}
		
		// Delete processed file
		unlink((workingFolderLocation + "/output.gcode").c_str());
		
		// Display message
		cout << outputFile << " was successfully created" << endl;
	}
	
	// Return true
	return true;
}

bool Printer::printFile(const char *file) {

	// Initialize variables
	fstream processedFile;
	string line, response;
	Gcode gcode;
	queue<string> buffer;
	char character;
	uint8_t commandsSent = 0;
	uint16_t lineNumber = 0;
	uint64_t totalLines = 0, lineCounter = 0;
	bool firstCommandSent = false;
	
	// Check if processing file failed
	if(!processFile(file)) {
	
		// Display error
		cout << "Processing file failed" << endl;
		return false;
	}
	
	// Go through the fully processed file
	processedFile.open(workingFolderLocation + "/output.gcode", ios::in | ios::binary);
	while(processedFile.peek() != EOF) {
	
		// Check if line contains valid g-code
		getline(processedFile, line);
		if(gcode.parseLine(line))
		
			// Increment total lines
			totalLines++;
	}
	
	// Add number of reset line number commands to total lines
	totalLines += 1 + totalLines / UINT16_MAX;
	
	// Go to the beginning of the fully processed file
	processedFile.clear();
	processedFile.seekg(0, ios::beg);
	
	// Display message
	cout << "Starting print" << endl << endl;

	// Go through all commands
	while(processedFile.peek() != EOF || commandsSent != 0) {
		
		// Check if new g-code can be sent
		if(processedFile.peek() != EOF && commandsSent <= 3) {
		
			// Check if first command hasn't been sent
			if(!firstCommandSent) {
		
				// Set command to starting line number
				line = "M110";
				
				// Set first command sent
				firstCommandSent = true;
			}
		
			// Otherwise
			else
		
				// Get line
				getline(processedFile, line);
		}
		
		// Otherwise
		else
		
			// Clear line
			line.clear();
		
		// Check if line contains valid g-code
		if(gcode.parseLine(line)) {
		
			// Set command's line number
			gcode.setValue('N', to_string(lineNumber++));
			
			// Send request
			sendRequest(gcode);
			
			// Append request to buffer
			buffer.push(gcode.getAscii());
		
			// Increment commands sent
			commandsSent++;
		}
		
		// Wait before checking for a response
		usleep(500);
		
		// Go through all responses if avaliable
		while(read(fd, &character, 1) != -1) {
		
			// Get response
			response = character + receiveResponse();
			
			// Check if response was a processed value or skip value
			if((response.length() >= 4 && response.substr(0, 2) == "ok" && response[3] >= '0' && response[3] <= '9') || (response.length() >= 6 && response.substr(0, 4) == "skip")) {
				
				// Display message
				cout << "Processed: " << buffer.front() << endl;
				
				// Remove command from buffer
				commandsSent--;
				lineCounter++;
				buffer.pop();
				
				// Display percent complete
				cout << dec << lineCounter << '/' << totalLines << ' ' << fixed << static_cast<float>(lineCounter) / totalLines * 100 << '%' << endl << endl;
			}
			
			// Otherwise check if response was a resend value
			else if(response.length() >= 8 && response.substr(0, 6) == "Resend") {
				
				// Resend request
				sendRequest(buffer.front());
				
			}
			
			// Wait before receiving next response
			usleep(500);
		}
	}
	
	// Close processed file
	processedFile.close();
	
	// Delete processed file
	unlink((workingFolderLocation + "/output.gcode").c_str());
	
	// Display message
	cout << "Print finished" << endl;
	
	// Return true
	return true;
}

void Printer::setValidationPreprocessor() {

	// Set use validation
	useValidation = true;
}

void Printer::setPreparationPreprocessor() {

	// Set use preparation
	usePreparation = true;
}

void Printer::setWaveBondingPreprocessor() {

	// Set use wave bonding
	useWaveBonding = true;
}

void Printer::setThermalBondingPreprocessor() {

	// Set use thermal bonding
	useThermalBonding = true;
}

void Printer::setBedCompensationPreprocessor() {

	// Set use bed compensation
	useBedCompensation = true;
}

void Printer::setBacklashCompensationPreprocessor() {

	// Set use backlash compensation
	useBacklashCompensation = true;
}

void Printer::setFeedRateConversionPreprocessor() {

	// Set use feed rate conversion
	useFeedRateConversion = true;
}

void Printer::setCenterModelPreprocessor() {

	// Set use center model
	useCenterModel = true;
}

void Printer::setBacklashX(const string &value) {

	// Set backlash X
	backlashX = stod(value);
}

void Printer::setBacklashY(const string &value) {

	// Set backlash Y
	backlashY = stod(value);
}

void Printer::setBacklashSpeed(const string &value) {

	// Set backlash speed
	backlashSpeed = stod(value);
}

void Printer::setBedHeightOffset(const string &value) {

	// Set bed height offset
	bedHeightOffset = stod(value);
}

void Printer::setBackRightOffset(const string &value) {

	// Set back right offset
	backRightOffset = stod(value);
}

void Printer::setBackLeftOffset(const string &value) {

	// Set back left offset
	backLeftOffset = stod(value);
}

void Printer::setFrontLeftOffset(const string &value) {

	// Set front left offset
	frontLeftOffset = stod(value);
}

void Printer::setFrontRightOffset(const string &value) {

	// Set front right offset
	frontRightOffset = stod(value);
}

void Printer::setFilamentTemperature(const string &value) {

	// Set filament temperature
	filamentTemperature = stoi(value);
}

void Printer::setFilamentType(const string &value) {

	// Set filament type
	if(value == "ABS")
		filamentType = ABS;
	else if(value == "PLA")
		filamentType = PLA;
	else if(value == "HIPS")
		filamentType = HIPS;
	else if(value == "OTHER")
		filamentType = OTHER;
	else
		filamentType = NO_TYPE;
}

void Printer::translatorMode() {

	// Initialize variables
	char character;
	string buffer;
	ifstream file;
	Gcode gcode;
	uint64_t numberWrapCounter = 0, lineNumber;
	
	// Check if creating virtual port failed
	if((vd = posix_openpt(O_RDWR | O_NONBLOCK)) == -1)
	
		// Return
		return;
	
	// Check if changing ownership failed
	if(grantpt(vd) == -1) {
	
		// Close virtual port
		close(vd);
		return;
	}
	
	// Check if unlocking failed
	if(unlockpt(vd) == -1) {
	
		// Close virtual port
		close(vd);
		return;
	}
	
	// Check if failed to get location
	if(ptsname(vd) == NULL) {
	
		// Close virtual port
		close(vd);
		return;
	}
	
	// Go through all serial device names
	for(uint16_t i = 0; i < UINT16_MAX && virtualSerialPortLocation.empty(); i++) {
	
		// Check if device name doesn't already exists
		file.open("/dev/ttyACM" + to_string(i));
		if(!file.good())
		
			// Set virtual serial port location
			virtualSerialPortLocation = "/dev/ttyACM" + to_string(i);
		
		// Close the file
		file.close();
	}
	
	// Check if no device names were avaliable
	if(virtualSerialPortLocation.empty()) {
	
		// Close virtual port
		close(vd);
		return;
	}
	
	// Check if creating a symbolic link between virtual port and serial port failed
	if(symlink(ptsname(vd), virtualSerialPortLocation.c_str()) == -1) {
	
		// Close virtual port
		close(vd);
		return;
	}
	
	// Check if changing permission of virtual port failed
	if(chmod(virtualSerialPortLocation.c_str(), S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH) == -1) {
	
		// Close virtual port
		close(vd);
		return;
	}
	
	// Display message
	cout << "Translation port established at " << virtualSerialPortLocation << endl;
	
	// Loop forever
	while(1) {
	
		// Check if data is being sent to the printer
		if(read(vd, &character, 1) == 1) {
		
			// Get request
			buffer.clear();
			do {
				buffer.push_back(character);
			} while(read(vd, &character, 1) == 1);
			
			// Check if request is unknown
			if(buffer == "M110\n" || buffer == "M21\n") {
			
				// Send expected response from the printer
				tcflush(vd, TCIOFLUSH);
				if(write(vd, "ok\n", 3) != 3)
					return;
				tcdrain(vd);
			}
			
			// Otherwise
			else {
			
				// Check if data contains valid g-code
				if(gcode.parseLine(buffer)) {
				
					// Set buffer to value
					buffer = gcode.getAscii();
					
					// Check if data contains a starting line number
					if(gcode.getValue('N') == "0" && gcode.getValue('M') == "110")
					
						// Reset number wrap counter
						numberWrapCounter = 0;
				}
				
				// Send request to the printer
				sendRequest(buffer);
			}
		}
		
		// Check if data is being sent from the printer
		if(read(fd, &character, 1) == 1) {
		
			// Get response
			buffer.clear();
			do {
				buffer.push_back(character);
			} while(read(fd, &character, 1) == 1);
			
			// Check if response was a processed value
			if(buffer.length() >= 4 && buffer.substr(0, 2) == "ok" && buffer[3] >= '0' && buffer[3] <= '9') {
			
				// Get line number
				lineNumber = stoi(buffer.substr(3));
			
				// Set buffer to contain correct line number
				buffer = "ok " + to_string(lineNumber + numberWrapCounter * 0x10000) + '\n';
				
				// Increment number wrap counter if applicable
				if(lineNumber == UINT16_MAX)
					numberWrapCounter++;
			}
			
			// Otherwise check if response was a skip value
			else if(buffer.length() >= 6 && buffer.substr(0, 4) == "skip") {
			
				// Get line number
				lineNumber = stoi(buffer.substr(5));
			
				// Set buffer to contain correct line number
				buffer = "ok " + to_string(lineNumber + numberWrapCounter * 0x10000) + '\n';
				
				// Increment number wrap counter if applicable
				if(lineNumber == UINT16_MAX)
					numberWrapCounter++;
			}
			
			// Otherwise check if response was a resend value
			else if(buffer.length() >= 8 && buffer.substr(0, 6) == "Resend")
			
				// Set buffer to contain correct line number
				buffer = "Resend:" + to_string(stoi(buffer.substr(7)) + numberWrapCounter * 0x10000) + '\n';
			
			// Send response from printer
			tcflush(vd, TCIOFLUSH);
			if(write(vd, buffer.c_str(), buffer.size()) != static_cast<unsigned int>(buffer.size()))
				return;
			tcdrain(vd);
		}
		
		// Wait
		usleep(200);
	}
	
	// Close virtual port
	close(vd);
}

bool Printer::sendRequestAscii(char data) {

	// Initialize variables
	bool returnValue;

	// Send data to the device
	tcflush(fd, TCIOFLUSH);
	returnValue = write(fd, &data, 1) != -1;
	tcdrain(fd);
	
	// Return value
	return returnValue;
}

bool Printer::sendRequestAscii(const char *data) {

	// Initialize variables
	bool returnValue;

	// Send data to the device
	tcflush(fd, TCIOFLUSH);
	returnValue = write(fd, data, strlen(data)) != -1;
	tcdrain(fd);
	
	// Return value
	return returnValue;
}

bool Printer::sendRequestAscii(const Gcode &data) {

	// Initialize variables
	bool returnValue;
	string request = data.getAscii();

	// Send data to the device
	tcflush(fd, TCIOFLUSH);
	returnValue = write(fd, request.c_str(), request.size()) != -1;
	tcdrain(fd);
	
	// Return value
	return returnValue;
}

bool Printer::sendRequestBinary(const char *data) {

	// Initialize variables
	bool returnValue;
	Gcode gcode;
	vector<uint8_t> request;
	
	// Check if line was successfully parsed
	if(gcode.parseLine(data)) {
	
		// Get binary data
		request = gcode.getBinary();
	
		// Send binary request to the device
		tcflush(fd, TCIOFLUSH);
		returnValue = write(fd, request.data(), request.size()) != -1;
		tcdrain(fd);
		
		// Set bootloader mode and reconnect if necessary
		bootloaderMode = gcode.getValue('M') == "115" && gcode.getValue('S') == "628";
		if(bootloaderMode)
			while(!connect());
		
		// Return value
		return returnValue;
	}
	
	// Return false
	return false;
}

bool Printer::sendRequestBinary(const Gcode &data) {

	// Initialize variables
	bool returnValue;
	vector<uint8_t> request = data.getBinary();
	
	// Send binary request to the device
	tcflush(fd, TCIOFLUSH);
	returnValue = write(fd, request.data(), request.size()) != -1;
	tcdrain(fd);
	
	// Set bootloader mode and reconnect if necessary
	bootloaderMode = data.getValue('M') == "115" && data.getValue('S') == "628";
	if(bootloaderMode)
		while(!connect());
	
	// Return value
	return returnValue;
}

string Printer::receiveResponseAscii() {

	// Initialize variables
	string response;
	char character;
	uint8_t i;
	
	// Wait 200 milliseconds for a response
	for(i = 0; i < 200 && read(fd, &character, 1) == -1; i++)
		usleep(1000);
	
	// Return an empty string if no response is received
	if(i == 200)
		return response;
	
	// Get response
	do {
		response.push_back(character);
		usleep(50);
	} while(read(fd, &character, 1) != -1);
	
	// Return response
	return response;
}

string Printer::receiveResponseBinary() {

	// Initialize variables
	string response;
	char character;
	uint8_t i;
	
	// Wait 200 ms for a response
	for(i = 0; i < 200 && read(fd, &character, 1) == -1; i++)
		usleep(1000);
	
	// Return an empty string if no response is received
	if(i == 200)
		return response;
	
	// Get response
	while(character != '\n') {
		response.push_back(character);
		while(read(fd, &character, 1) == -1);
	}
	
	// Return response
	return response;
}

bool Printer::writeToEeprom(uint16_t address, const uint8_t *data, uint16_t length) {

	// Send write to eeprom request
	sendRequestAscii('U');
	sendRequestAscii(address >> 8);
	sendRequestAscii(address);
	sendRequestAscii(length >> 8);
	sendRequestAscii(length);
	
	// Send data
	for(uint16_t i = 0; i < length; i++)
		sendRequestAscii(data[i]);
	
	// Return if write was successful
	return receiveResponseAscii() == "\r";
}

bool Printer::writeToEeprom(uint16_t address, uint8_t data) {

	// Return if write was successful
	return writeToEeprom(address, &data, 1);
}

uint32_t Printer::crc32(int32_t offset, const uint8_t *data, int32_t count) {

	// Initialize variables
	uint32_t crc = 0;
	
	// Update crc
	crc ^= crc32Seed;

	// Go through data
	while(--count >= 0)
	
		// Update crc
		crc = crc32Table[(crc ^ data[offset++]) & 0xFF] ^ (crc >> 8);
	
	// Return updated crc
	return crc ^ crc32Seed;
}

bool Printer::createSettingsFile() {

	// Initialize variables
	ofstream file;
	char userName[256];
	passwd *pwd;
	struct stat buffer;
	
	// Check if creating folder failed if it doesn't already exists
	if(stat("/usr/share/m33-linux", &buffer) == -1 && mkdir("/usr/share/m33-linux", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
		return false;
	
	// Create settings file
	file.open("/usr/share/m33-linux/settings", ios::out | ios::binary);
	
	// Check if creating settings file was successful
	if(file.good()) {
	
		// Write values to file
		file << "Back Right Offset: " << backRightOffset << endl;
		file << "Back Left Offset: " << backLeftOffset << endl;
		file << "Front Left Offset: " << frontLeftOffset << endl;
		file << "Front Right Offset: " << frontRightOffset << endl;
		file << "Bed Height Offset: " << bedHeightOffset << endl;
		file << "Backlash X: " << backlashX << endl;
		file << "Backlash Y: " << backlashY << endl;
		file << "Backlash Speed: " << backlashSpeed << endl;
		file << "Back Right Orientation: " << backRightOrientation << endl;
		file << "Back Left Orientation: " << backLeftOrientation << endl;
		file << "Front Left Orientation: " << frontLeftOrientation << endl;
		file << "Front Right Orientation: " << frontRightOrientation << endl;
		file << "Filament Location: " << filamentLocation << endl;
		file << "Filament Type: " << filamentType << endl;
		file << "Filament Color: " << filamentColor<< endl;
		file << "Filament Temperature: " << filamentTemperature;
		
		// Close file
		file.close();
		
		// Check if getting user name was successful
		if(getlogin_r(userName, 256))
		
			// Return false
			return false;
		
		// Check if getting group or user id failed
		if((pwd = getpwnam(userName)) == NULL)
		
			// Return false
			return false;
		
		// Check if changing permission of file failed
		if(chmod("/usr/share/m33-linux/settings", S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR) == -1) {
		
			// Delete output file
			unlink("/usr/share/m33-linux/settings");
			return false;
		}
		
		// Check if changing ownership of file failed
		if(chown("/usr/share/m33-linux/settings", pwd->pw_uid, pwd->pw_gid) == -1) {
		
			// Delete output file
			unlink("/usr/share/m33-linux/settings");
			return false;
		}
	
		// Return true
		return true;
	}
	
	// Return false
	return false;
}

bool Printer::useSettingsFile() {

	// Initialize variables
	string line;
	ifstream file("/usr/share/m33-linux/settings", ios::in | ios::binary);
	
	// Check if creating settings file was successful
	if(file.good()) {
	
		// Go through entire file
		while(file.peek() != EOF) {
		
			// Get line
			getline(file, line);
			
			// Catch string errors
			try {
			
				// Assign value based on line content
				if(line.find("Back Right Offset") != string::npos)
					backRightOffset = stod(line.substr(line.find(':') + 1));
				
				else if(line.find("Back Left Offset") != string::npos)
					backLeftOffset = stod(line.substr(line.find(':') + 1));
				
				else if(line.find("Front Left Offset") != string::npos)
					frontLeftOffset = stod(line.substr(line.find(':') + 1));
				
				else if(line.find("Front Right Offset") != string::npos)
					frontRightOffset = stod(line.substr(line.find(':') + 1));
				
				else if(line.find("Bed Height Offset") != string::npos)
					bedHeightOffset = stod(line.substr(line.find(':') + 1));
				
				else if(line.find("Backlash X") != string::npos)
					backlashX = stod(line.substr(line.find(':') + 1));
				
				else if(line.find("Backlash Y") != string::npos)
					backlashY = stod(line.substr(line.find(':') + 1));
				
				else if(line.find("Backlash Speed") != string::npos)
					backlashSpeed = stod(line.substr(line.find(':') + 1));
				
				else if(line.find("Back Right Orientation") != string::npos)
					backRightOrientation = stod(line.substr(line.find(':') + 1));
				
				else if(line.find("Back Left Orientation") != string::npos)
					backLeftOrientation = stod(line.substr(line.find(':') + 1));
				
				else if(line.find("Front Left Orientation") != string::npos)
					frontLeftOrientation = stod(line.substr(line.find(':') + 1));
				
				else if(line.find("Front Right Orientation") != string::npos)
					frontRightOrientation = stod(line.substr(line.find(':') + 1));
				
				else if(line.find("Filament Location") != string::npos)
					filamentLocation = static_cast<filamentLocations>(stoi(line.substr(line.find(':') + 1)));
				
				else if(line.find("Filament Type") != string::npos)
					filamentType = static_cast<filamentTypes>(stoi(line.substr(line.find(':') + 1)));
				
				else if(line.find("Filament Color") != string::npos)
					filamentColor = static_cast<filamentColors>(stoi(line.substr(line.find(':') + 1)));
				
				else if(line.find("Filament Temperature") != string::npos)
					filamentTemperature = stoi(line.substr(line.find(':') + 1));
			}
			
			// Check if an out of range error has occured
			catch(const out_of_range& exception) {
			
				// Close file
				file.close();
	
				// Return false
				return false;
			}
	
			// Check if an invalid argument error has occured
			catch(const invalid_argument& exception) {
			
				// Close file
				file.close();
	
				// Return false
				return false;
			}
		}
		
		// Close file
		file.close();
	
		// Return true
		return true;
	}
	
	// Return false
	return false;
}

double Printer::max(double first, double second) {

	// Return larger of the two
	return first > second ? first : second;
}

uint16_t Printer::getBoundedTemperature(uint16_t temperature) {

	// Return temperature bounded by range
	return temperature > 285 ? 285 : temperature < 150 ? 150 : temperature;
}

double Printer::getDistance(const Gcode &firstPoint, const Gcode &secondPoint) {

	// Get first point coordinates
	double firstX = firstPoint.hasValue('X') ? stod(firstPoint.getValue('X')) : 0;
	double firstY = firstPoint.hasValue('Y') ? stod(firstPoint.getValue('Y')) : 0;
	
	// Get second point coordinates
	double secondX = secondPoint.hasValue('X') ? stod(secondPoint.getValue('X')) : 0;
	double secondY = secondPoint.hasValue('Y') ? stod(secondPoint.getValue('Y')) : 0;

	// Return distance between the two values
	return sqrt(pow(firstX - secondX, 2) + pow(firstY - secondY, 2));
}

Gcode Printer::createTackPoint(const Gcode &point, const Gcode &refrence) {

	// Initialize variables
	Gcode gcode;
	uint16_t time = ceil(getDistance(point, refrence));
	
	// Check if time is greater than 5
	if(time > 5) {
	
		// Set g-code to a delay command based on time
		gcode.setValue('G', "4");
		gcode.setValue('P', to_string(time));
	}
	
	// Return gcode
	return gcode;
}

bool Printer::isSharpCorner(const Gcode &point, const Gcode &refrence) {

	// Initialize variables
	double value;
	
	// Get point coordinates
	double currentX = point.hasValue('X') ? stod(point.getValue('X')) : 0;
	double currentY = point.hasValue('Y') ? stod(point.getValue('Y')) : 0;
	
	// Get refrence coordinates
	double previousX = refrence.hasValue('X') ? stod(refrence.getValue('X')) : 0;
	double previousY = refrence.hasValue('Y') ? stod(refrence.getValue('Y')) : 0;
	
	// Calculate value
	if((!currentX && !currentY) || (!previousX && !previousY))
		value = acos(0);
	else
		value = acos((currentX * previousX + currentY * previousY) / (pow(currentX * currentX + currentY * currentY, 2) * pow(previousX * previousX + previousY * previousY, 2)));
	
	// Return if sharp corner
	return value > 0 && value < M_PI_2;
}

double Printer::getCurrentAdjustmentZ() {

	// Initialize variables
	static uint8_t waveStep = 0;

	// Set adjustment
	double adjustment = waveStep ? waveStep != 2 ? 0 : -1.5 : 1;
	
	// Increment wave step
	waveStep = (waveStep + 1) % 4;
	
	// Return adjustment
	return adjustment * WAVE_SIZE;
}

Vector Printer::calculatePlaneNormalVector(const Vector &v1, const Vector &v2, const Vector &v3) {

	// Initialize variables
	Vector vector, vector2, vector3;
	vector = v2 - v1;
	vector2 = v3 - v1;
	
	// Return normal vector
	vector3[0] = vector[1] * vector2[2] - vector2[1] * vector[2];
	vector3[1] = vector[2] * vector2[0] - vector2[2] * vector[0];
	vector3[2] = vector[0] * vector2[1] - vector2[0] * vector[1];
	return vector3;
}

Vector Printer::generatePlaneEquation(const Vector &v1, const Vector &v2, const Vector &v3) {

	// Initialize variables
	Vector vector, vector2;
	vector2 = calculatePlaneNormalVector(v1, v2, v3);
	
	// Return plane equation
	vector[0] = vector2[0];
	vector[1] = vector2[1];
	vector[2] = vector2[2];
	vector[3] = -(vector[0] * v1[0] + vector[1] * v1[1] + vector[2] * v1[2]);
	return vector;
}

double Printer::getHeightAdjustmentRequired(double x, double y) {

	// Set corner vectors
	Vector vector(99, 95, backRightOrientation + backRightOffset);
	Vector vector2(9, 95, backLeftOrientation + backLeftOffset);
	Vector vector3(9, 5, frontLeftOrientation + frontLeftOffset);
	Vector vector4(99, 5, frontRightOrientation + frontRightOffset);
	Vector vector5(54, 50, 0);
	
	// Calculate planes
	Vector planeABC, vector7, vector8, vector9;
	planeABC = generatePlaneEquation(vector2, vector, vector5);
	vector7 = generatePlaneEquation(vector2, vector3, vector5);
	vector8 = generatePlaneEquation(vector, vector4, vector5);
	vector9 = generatePlaneEquation(vector3, vector4, vector5);
	Vector point(x, y, 0);
	
	// Return height adjustment
	if(x <= vector3.x && y >= vector.y)
		return (getZFromXYAndPlane(point, planeABC) + getZFromXYAndPlane(point, vector7)) / 2;
	
	else if(x <= vector3.x && y <= vector3.y)
		return (getZFromXYAndPlane(point, vector9) + getZFromXYAndPlane(point, vector7)) / 2;
	
	else if(x >= vector4.x && y <= vector3.y)
		return (getZFromXYAndPlane(point, vector9) + getZFromXYAndPlane(point, vector8)) / 2;
	
	else if(x >= vector4.x && y >= vector.y)
		return (getZFromXYAndPlane(point, planeABC) + getZFromXYAndPlane(point, vector8)) / 2;
	
	else if(x <= vector3.x)
		return getZFromXYAndPlane(point, vector7);
	
	else if(x >= vector4.x)
		return getZFromXYAndPlane(point, vector8);
	
	else if(y >= vector.y)
		return getZFromXYAndPlane(point, planeABC);
	
	else if(y <= vector3.y)
		return getZFromXYAndPlane(point, vector9);
	
	else if(isPointInTriangle(point, vector5, vector3, vector2))
		return getZFromXYAndPlane(point, vector7);
	
	else if(isPointInTriangle(point, vector5, vector4, vector))
		return getZFromXYAndPlane(point, vector8);
	
	else if(isPointInTriangle(point, vector5, vector2, vector))
		return getZFromXYAndPlane(point, planeABC);
	
	else
		return getZFromXYAndPlane(point, vector9);
}

double Printer::getZFromXYAndPlane(const Vector &point, const Vector &planeABC) {

	// Return Z
	return (planeABC[0] * point.x + planeABC[1] * point.y + planeABC[3]) / -planeABC[2];
}

bool Printer::isPointInTriangle(const Vector &pt, const Vector &v1, const Vector &v2, const Vector &v3) {

	// Initialize variables
	Vector vector, vector2, vector3, vector4;
	vector = v1 - v2 + v1 - v3;
	vector.normalize();
	vector2 = v1 + vector * 0.01;
	vector = v2 - v1 + v2 - v3;
	vector.normalize();
	vector3 = v2 + vector * 0.01;
	vector = v3 - v1 + v3 - v2;
	vector.normalize();
	vector4 = v3 + vector * 0.01;
	
	// Return if inside triangle
	bool flag = sign(pt, vector2, vector3) < 0;
	bool flag2 = sign(pt, vector3, vector4) < 0;
	bool flag3 = sign(pt, vector4, vector2) < 0;
	return flag == flag2 && flag2 == flag3;
}

double Printer::sign(const Vector &p1, const Vector &p2, const Vector &p3) {

	// Return sign
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool Printer::checkPrintDimensions(const char *file, bool overrideCenterModelPreprocessor) {

	// Check if using center model pre-processor
	if(!overrideCenterModelPreprocessor && useCenterModel)
	
		// Return if adjusted print values are within bounds
		return minZExtruder >= BED_LOW_MIN_Z && maxZExtruder <= BED_HIGH_MAX_Z && maxXExtruderLow <= BED_LOW_MAX_X && maxXExtruderMedium <= BED_MEDIUM_MAX_X && maxXExtruderHigh <= BED_HIGH_MAX_X && maxYExtruderLow <= BED_LOW_MAX_Y && maxYExtruderMedium <= BED_MEDIUM_MAX_Y && maxYExtruderHigh <= BED_HIGH_MAX_Y && minXExtruderLow >= BED_LOW_MIN_X && minXExtruderMedium >= BED_MEDIUM_MIN_X && minXExtruderHigh >= BED_HIGH_MIN_X && minYExtruderLow >= BED_LOW_MIN_Y && minYExtruderMedium >= BED_MEDIUM_MIN_Y && minYExtruderHigh >= BED_HIGH_MIN_Y;
		
	// Initialize file
	ifstream input(file, ios::in | ios::binary);

	// Check if input file was opened successfully
	if(input.good()) {

		// Initialize variables
		string line;
		Gcode gcode;
		printTiers tier = LOW;
		bool relativeMode = false;
		double localX = 54, localY = 50, localZ = 0.4;
		double commandX, commandY, commandZ;
	
		// Reset all print values
		maxXExtruderLow = 0;
		maxXExtruderMedium = 0;
		maxXExtruderHigh = 0;
		maxYExtruderLow = 0;
		maxYExtruderMedium = 0;
		maxYExtruderHigh = 0;
		maxZExtruder = 0;
		minXExtruderLow = DBL_MAX;
		minXExtruderMedium = DBL_MAX;
		minXExtruderHigh = DBL_MAX;
		minYExtruderLow = DBL_MAX;
		minYExtruderMedium = DBL_MAX;
		minYExtruderHigh = DBL_MAX;
		minZExtruder = DBL_MAX;

		// Go through input file
		while(input.peek() != EOF) {
		
			// Read in a line
			getline(input, line);
			
			// Check if line was parsed successfully and it's a G command
			if(gcode.parseLine(line) && gcode.hasValue('G')) {
		
				// Check what parameter is associated with the command
				switch(stoi(gcode.getValue('G'))) {
				
					// G0 or G1
					case 0:
					case 1:
					
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
							
							// Check if not ignoring print dimension limitations and Z is out of bounds
							if(!ignorePrintDimensionLimitations && (localZ < BED_LOW_MIN_Z || localZ > BED_HIGH_MAX_Z))
					
								// Return false
								return false;
						
							// Set print tier
							if(localZ < BED_LOW_MAX_Z)
								tier = LOW;
							
							else if(localZ < BED_MEDIUM_MAX_Z)
								tier = MEDIUM;
							
							else
								tier = HIGH;
						}
					
						// Update minimums and maximums dimensions of extruder			
						switch(tier) {
					
							case LOW:
							
								// Check if not ignoring print dimension limitations and X or Y is out of bounds
								if(!ignorePrintDimensionLimitations && (localX < BED_LOW_MIN_X || localX > BED_LOW_MAX_X || localY < BED_LOW_MIN_Y || localY > BED_LOW_MAX_Y))
								
									// Return false
									return false;
						
								minXExtruderLow = minXExtruderLow < localX ? minXExtruderLow : localX;
								maxXExtruderLow = maxXExtruderLow > localX ? maxXExtruderLow : localX;
								minYExtruderLow = minYExtruderLow < localY ? minYExtruderLow : localY;
								maxYExtruderLow = maxYExtruderLow > localY ? maxYExtruderLow : localY;
							break;
						
							case MEDIUM:
							
								// Check if not ignoring print dimension limitations and X or Y is out of bounds
								if(!ignorePrintDimensionLimitations && (localX < BED_MEDIUM_MIN_X || localX > BED_MEDIUM_MAX_X || localY < BED_MEDIUM_MIN_Y || localY > BED_MEDIUM_MAX_Y))
								
									// Return false
									return false;
						
								minXExtruderMedium = minXExtruderMedium < localX ? minXExtruderMedium : localX;
								maxXExtruderMedium = maxXExtruderMedium > localX ? maxXExtruderMedium : localX;
								minYExtruderMedium = minYExtruderMedium < localY ? minYExtruderMedium : localY;
								maxYExtruderMedium = maxYExtruderMedium > localY ? maxYExtruderMedium : localY;
							break;

							case HIGH:
							
								// Check if not ignoring print dimension limitations and X or Y is out of bounds
								if(!ignorePrintDimensionLimitations && (localX < BED_HIGH_MIN_X || localX > BED_HIGH_MAX_X || localY < BED_HIGH_MIN_Y || localY > BED_HIGH_MAX_Y))
								
									// Return false
									return false;
						
								minXExtruderHigh = minXExtruderHigh < localX ? minXExtruderHigh : localX;
								maxXExtruderHigh = maxXExtruderHigh > localX ? maxXExtruderHigh : localX;
								minYExtruderHigh = minYExtruderHigh < localY ? minYExtruderHigh : localY;
								maxYExtruderHigh = maxYExtruderHigh > localY ? maxYExtruderHigh : localY;
							break;
						}
						
						minZExtruder = minZExtruder < localZ ? minZExtruder : localZ;
						maxZExtruder = maxZExtruder > localZ ? maxZExtruder : localZ;
					break;
					
					// G90
					case 90:
				
						// Clear relative mode
						relativeMode = false;
					break;
					
					// G91
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

bool Printer::centerModelPreprocessor(const char *file) {

	// Initialize temporary name
	string tempName = tmpnam(NULL);
	
	// Check if moving the input file to a temporary file was successful
	if(!rename(file, tempName.c_str())) {
		
		// Initialize files
		ifstream input(tempName, ios::in | ios::binary);
		ofstream output(file, ios::out | ios::binary);
	
		// Check if input and output files were opened successfully
		if(input.good() && output.good()) {
		
			// Initialize variables
			string line;
			Gcode gcode;
			printTiers tier = LOW;
			bool relativeMode = false;
			double localX = 54, localY = 50, localZ = 0.4;
			double commandX, commandY, commandZ;
			double displacementX, displacementY;
		
			// Reset all print values
			maxXExtruderLow = 0;
			maxXExtruderMedium = 0;
			maxXExtruderHigh = 0;
			maxYExtruderLow = 0;
			maxYExtruderMedium = 0;
			maxYExtruderHigh = 0;
			maxZExtruder = 0;
			minXExtruderLow = DBL_MAX;
			minXExtruderMedium = DBL_MAX;
			minXExtruderHigh = DBL_MAX;
			minYExtruderLow = DBL_MAX;
			minYExtruderMedium = DBL_MAX;
			minYExtruderHigh = DBL_MAX;
			minZExtruder = DBL_MAX;
	
			// Go through input file
			while(input.peek() != EOF) {
			
				// Read in a line
				getline(input, line);
				
				// Check if line was parsed successfully and it's a G command
				if(gcode.parseLine(line) && gcode.hasValue('G')) {
			
					// Check what parameter is associated with the command
					switch(stoi(gcode.getValue('G'))) {
					
						// G0 or G1
						case 0:
						case 1:
						
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
							
								// Set print tier
								if(localZ < BED_LOW_MAX_Z)
									tier = LOW;
								
								else if(localZ < BED_MEDIUM_MAX_Z)
									tier = MEDIUM;
								
								else
									tier = HIGH;
							}
						
							// Update minimums and maximums dimensions of extruder			
							switch(tier) {
						
								case LOW:
							
									minXExtruderLow = minXExtruderLow  < localX ? minXExtruderLow  : localX;
									maxXExtruderLow  = maxXExtruderLow  > localX ? maxXExtruderLow  : localX;
									minYExtruderLow  = minYExtruderLow  < localY ? minYExtruderLow  : localY;
									maxYExtruderLow  = maxYExtruderLow  > localY ? maxYExtruderLow  : localY;
								break;
							
								case MEDIUM:
							
									minXExtruderMedium = minXExtruderMedium  < localX ? minXExtruderMedium  : localX;
									maxXExtruderMedium  = maxXExtruderMedium  > localX ? maxXExtruderMedium  : localX;
									minYExtruderMedium  = minYExtruderMedium  < localY ? minYExtruderMedium  : localY;
									maxYExtruderMedium  = maxYExtruderMedium  > localY ? maxYExtruderMedium  : localY;
								break;

								case HIGH:
							
									minXExtruderHigh = minXExtruderHigh  < localX ? minXExtruderHigh  : localX;
									maxXExtruderHigh  = maxXExtruderHigh  > localX ? maxXExtruderHigh  : localX;
									minYExtruderHigh  = minYExtruderHigh  < localY ? minYExtruderHigh  : localY;
									maxYExtruderHigh  = maxYExtruderHigh  > localY ? maxYExtruderHigh  : localY;
								break;
							}
							
							minZExtruder = minZExtruder < localZ ? minZExtruder : localZ;
							maxZExtruder = maxZExtruder > localZ ? maxZExtruder : localZ;
						break;
						
						// G90
						case 90:
					
							// Clear relative mode
							relativeMode = false;
						break;
						
						// G91
						case 91:
					
							// Set relative mode
							relativeMode = true;
						break;
					}
				}
			}
			
			// Calculate adjustments
			displacementX = (BED_LOW_MAX_X - max(maxXExtruderLow, max(maxXExtruderMedium, maxXExtruderHigh)) - min(minXExtruderLow, min(minXExtruderMedium, minXExtruderHigh)) + BED_LOW_MIN_X) / 2;
			displacementY = (BED_LOW_MAX_Y - max(maxYExtruderLow, max(maxYExtruderMedium, maxYExtruderHigh)) - min(minYExtruderLow, min(minYExtruderMedium, minYExtruderHigh)) + BED_LOW_MIN_Y) / 2;
		
			// Adjust print values
			maxXExtruderLow += displacementX;
			maxXExtruderMedium += displacementX;
			maxXExtruderHigh += displacementX;
			maxYExtruderLow += displacementY;
			maxYExtruderMedium += displacementY;
			maxYExtruderHigh += displacementY;
			minXExtruderLow += displacementX;
			minXExtruderMedium += displacementX;
			minXExtruderHigh += displacementX;
			minYExtruderLow += displacementY;
			minYExtruderMedium += displacementY;
			minYExtruderHigh += displacementY;
			
			// Go back to the beginning of the input file
			input.clear();
			input.seekg(0, ios::beg);
			
			// Go through input file
			while(input.peek() != EOF) {
			
				// Read in a line
				getline(input, line);
				
				// Check if line was parsed successfully and it's a G command
				if(gcode.parseLine(line) && gcode.hasValue('G')) {
				
					// Check if line contains an X value
					if(gcode.hasValue('X'))
					
						// Adjust X value
						gcode.setValue('X', to_string(stod(gcode.getValue('X')) + displacementX));
				
					// Check if line contains a Y value
					if(gcode.hasValue('Y'))
					
						// Adjust Y value
						gcode.setValue('Y', to_string(stod(gcode.getValue('Y')) + displacementY));
				}
				
				// Send line to output file
				output << gcode << endl;
			}
		
			// Return if input file was successfully removed
			return !unlink(tempName.c_str());
		}
	}
	
	// Return false
	return false;
}

bool Printer::validationPreprocessor(const char *file) {

	// Initialize temporary name
	string tempName = tmpnam(NULL);
	
	// Check if moving the input file to a temporary file was successful
	if(!rename(file, tempName.c_str())) {
		
		// Initialize files
		ifstream input(tempName, ios::in | ios::binary);
		ofstream output(file, ios::out | ios::binary);
	
		// Check if input and output files were opened successfully
		if(input.good() && output.good()) {
		
			// Initialize variables
			string line;
			Gcode gcode;
	
			// Go through input file
			while(input.peek() != EOF) {
			
				// Read in a line
				getline(input, line);
				
				// Check if line contains valid G-code
				if(gcode.parseLine(line)) {
				
					// Check if command isn't valid for the printer
					if((gcode.hasValue('M') && (gcode.getValue('M') == "82" || gcode.getValue('M') == "83")) || (gcode.hasValue('G') && (gcode.getValue('G') == "21")))
			
						// Get next line
						continue;
			
					// Check if command contains tool selection
					if(gcode.hasParameter('T'))
			
						// Remove tool selection
						gcode.removeParameter('T');
				}
				
				// Send line to output file
				output << gcode << endl;
			}
		
			// Return if input file was successfully removed
			return !unlink(tempName.c_str());
		}
	}
	
	// Return false
	return false;
}

bool Printer::preparationPreprocessor(const char *file, bool overrideCornerExcess) {

	// Initialize temporary name
	string tempName = tmpnam(NULL);
	
	// Check if moving the input file to a temporary file was successful
	if(!rename(file, tempName.c_str())) {
		
		// Initialize files
		ifstream input(tempName, ios::in | ios::binary);
		ofstream output(file, ios::out | ios::binary);
	
		// Check if input and output files were opened successfully
		if(input.good() && output.good()) {
		
			// Initialize variables
			string line;
			Gcode gcode;
			double cornerX = 0, cornerY = 0;
			
			// Check if leaving excess at corner
			if(!overrideCornerExcess) {
			
				// Set corner X
				if(maxXExtruderLow < BED_LOW_MAX_X)
					cornerX = (BED_LOW_MAX_X - BED_LOW_MIN_X) / 2;
				else if(minXExtruderLow > BED_LOW_MIN_X)
					cornerX = -(BED_LOW_MAX_X - BED_LOW_MIN_X) / 2;
		
				// Set corner Y
				if(maxYExtruderLow < BED_LOW_MAX_Y)
					cornerY = (BED_LOW_MAX_Y - BED_LOW_MIN_Y - 10) / 2;
				else if(minYExtruderLow > BED_LOW_MIN_Y)
					cornerY = -(BED_LOW_MAX_Y - BED_LOW_MIN_Y - 10) / 2;
			}
			
			// Add intro to output
			output << "M106 S" << (filamentType == PLA ? "255" : "50") << endl;
			output << "M17" << endl;
			output << "G90" << endl;
			output << "M104 S" << to_string(filamentTemperature) << endl;
			output << "G0 Z5 F2900" << endl;
			output << "G28" << endl;
		
			// Check if one of the corners wasn't set
			if(cornerX == 0 || cornerY == 0) {
			
				// Prepare extruder the standard way
				output << "M18" << endl;
				output << "M109 S" << to_string(filamentTemperature) << endl;
				output << "G4 S2" << endl;
				output << "M17" << endl;
				output << "G91" << endl;
			}
		
			// Otherwise
			else {
		
				// Prepare extruder by leaving excess at corner
				output << "G91" << endl;
				output << "G0 X" << to_string(-cornerX) << " Y" << to_string(-cornerY) << " F2900" << endl;
				output << "M18" << endl;
				output << "M109 S" << to_string(filamentTemperature) << endl;
				output << "M17" << endl;
				output << "G0 Z-4 F2900" << endl;
				output << "G0 E7.5 F2000" << endl;
				output << "G4 S3" << endl;
				output << "G0 X" << to_string(cornerX * 0.1) << " Y" << to_string(cornerY * 0.1) << " Z-0.999 F2900" << endl;
				output << "G0 X" << to_string(cornerX * 0.9) << " Y" << to_string(cornerY * 0.9) << " F1000" << endl;
			}
		
			output << "G92 E0" << endl;
			output << "G90" << endl;
			output << "G0 Z0.4 F2400" << endl;
			
			// Send input to output
			output << input.rdbuf();
			
			// Add outro to output
			output << "G91" << endl;
			output << "G0 E-1 F2000" << endl;
			output << "G0 X5 Y5 F2000" << endl;
			output << "G0 E-8 F2000" << endl;
			output << "M104 S0" << endl;
			if(maxZExtruder > 60) {
				if(maxZExtruder < 110)
					output << "G0 Z3 F2900" << endl;
				output << "G90" << endl;
				output << "G0 X90 Y84" << endl;
			}
			else {
				output << "G0 Z3 F2900" << endl;
				output << "G90" << endl;
				output << "G0 X95 Y95" << endl;
			}
			output << "M18" << endl;
			output << "M107" << endl;
		
			// Return if input file was successfully removed
			return !unlink(tempName.c_str());
		}
	}
	
	// Return false
	return false;
}

bool Printer::waveBondingPreprocessor(const char *file) {

	// Initialize temporary name
	string tempName = tmpnam(NULL);
	
	// Check if moving the input file to a temporary file was successful
	if(!rename(file, tempName.c_str())) {
		
		// Initialize files
		ifstream input(tempName, ios::in | ios::binary);
		ofstream output(file, ios::out | ios::binary);
	
		// Check if input and output files were opened successfully
		if(input.good() && output.good()) {
		
			// Initialize variables
			string line;
			Gcode gcode, previousGcode, refrenceGcode, tackPoint, extraGcode;
			bool relativeMode = false, changesPlane = false;
			uint32_t cornerCounter = 0, layerCounter = 0, waveRatio;
			double distance;
			double positionRelativeX = 0, positionRelativeY = 0, positionRelativeZ = 0, positionRelativeE = 0;
			double deltaX, deltaY, deltaZ, deltaE;
			double tempRelativeX, tempRelativeY, tempRelativeZ, tempRelativeE;
			double relativeDifferenceX, relativeDifferenceY, relativeDifferenceZ, relativeDifferenceE;
			double deltaRatioX, deltaRatioY, deltaRatioZ, deltaRatioE;
	
			// Go through input file
			while(input.peek() != EOF) {
			
				// Read in a line
				getline(input, line);
				
				// Check if line is a layer command
				if(line.find(";LAYER:") != string::npos)
				
					// Increment layer counter
					layerCounter++;
			
				// Check if line was parsed successfully, it's on the first layer, and it's a G command
				if(gcode.parseLine(line) && layerCounter == 1 && gcode.hasValue('G')) {
				
					// Check if command is G0 or G1 and it's in absolute mode
					if((gcode.getValue('G') == "0" || gcode.getValue('G') == "1") && !relativeMode) {
			
						// Check if line contains an X or Y value
						if(gcode.hasValue('X') || gcode.hasValue('Y'))
				
							// Set changes plane
							changesPlane = true;
					
						// Set delta values
						deltaX = !gcode.hasValue('X') ? 0 : stod(gcode.getValue('X')) - positionRelativeX;
						deltaY = !gcode.hasValue('Y')? 0 : stod(gcode.getValue('Y')) - positionRelativeY;
						deltaZ = !gcode.hasValue('Z') ? 0 : stod(gcode.getValue('Z')) - positionRelativeZ;
						deltaE = !gcode.hasValue('E') ? 0 : stod(gcode.getValue('E')) - positionRelativeE;
				
						// Adjust relative values for the changes
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
						if(distance) {
							deltaRatioX = deltaX / distance;
							deltaRatioY = deltaY / distance;
							deltaRatioZ = deltaZ / distance;
							deltaRatioE = deltaE / distance;
						}
						else {
							deltaRatioX = 0;
							deltaRatioY = 0;
							deltaRatioZ = 0;
							deltaRatioE = 0;
						}
				
						// Check if delta E is greater than zero
						if(deltaE > 0) {
				
							// Check if previous g-code is not empty
							if(!previousGcode.isEmpty()) {
					
								// Check if corner count is at most one and sharp corner
								if(cornerCounter <= 1 && isSharpCorner(gcode, previousGcode)) {
						
									// Check if refrence g-codes isn't set
									if(refrenceGcode.isEmpty()) {
							
										// Check if a tack point was created
										tackPoint = createTackPoint(gcode, previousGcode);
										if(!tackPoint.isEmpty())
								
											// Send tack point to output
											output << tackPoint << endl; 
									}
							
									// Set refrence g-code
									refrenceGcode = gcode;
							
									// Increment corner counter
									cornerCounter++;
								}
						
								// Otherwise check is corner count is at least one and sharp corner
								else if(cornerCounter >= 1 && isSharpCorner(gcode, refrenceGcode)) {
						
									// Check if a tack point was created
									tackPoint = createTackPoint(gcode, refrenceGcode);
									if(!tackPoint.isEmpty())
							
										// Send tack point to output
										output << tackPoint << endl; 
							
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
							
									// Set extra g-code F value if first element
									if(gcode.hasValue('F') && i == 1)
										extraGcode.setValue('F', gcode.getValue('F'));
							
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
							
									// Send extra g-code to output
									output << extraGcode << endl;
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
					
					// Otherwise check if command is G28
					else if(gcode.getValue('G') == "28") {
				
						// Set relative values
						positionRelativeX = 54;
						positionRelativeY = 50;
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
				
							// Set position relative values
							if(gcode.hasValue('X'))
								positionRelativeX = stod(gcode.getValue('X'));
							if(gcode.hasValue('Y'))
								positionRelativeY = stod(gcode.getValue('Y'));
							if(gcode.hasValue('Z'))
								positionRelativeZ = stod(gcode.getValue('Z'));
							if(gcode.hasValue('E'))
								positionRelativeE = stod(gcode.getValue('E'));
						}
					}
				}
				
				// Send line to output file
				output << gcode << endl;
			}
		
			// Return if input file was successfully removed
			return !unlink(tempName.c_str());
		}
	}
	
	// Return false
	return false;
}

bool Printer::thermalBondingPreprocessor(const char *file, bool overrideWaveBondingPreprocessor) {

	// Initialize temporary name
	string tempName = tmpnam(NULL);
	
	// Check if moving the input file to a temporary file was successful
	if(!rename(file, tempName.c_str())) {
		
		// Initialize files
		ifstream input(tempName, ios::in | ios::binary);
		ofstream output(file, ios::out | ios::binary);
	
		// Check if input and output files were opened successfully
		if(input.good() && output.good()) {
		
			// Initialize variables
			string line;
			Gcode gcode, previousGcode, refrenceGcode, tackPoint;
			uint32_t layerCounter = 0, cornerCounter = 0;
			bool relativeMode = false;
	
			// Go through input file
			while(input.peek() != EOF) {
			
				// Read in a line
				getline(input, line);
				
				// Check if line is a layer command
				if(layerCounter < 2 && line.find(";LAYER:") != string::npos) {
			
					// Check if on first counted layer
					if(layerCounter == 0)
					
						// Send temperature command to output
						output << "M109 S" << to_string(getBoundedTemperature(filamentTemperature + (filamentType == PLA ? 10 : 15))) << endl;
					
					// Otherwise
					else
						// Send temperature command to output
						output << "M104 S" << to_string(filamentTemperature) << endl;
				
					// Increment layer counter
					layerCounter++;
				}
			
				// Check if line was parsed successfully
				if(gcode.parseLine(line)) {
				
					// Check if command contains temperature or fan controls past the first layer
					if(layerCounter > 0 && gcode.hasValue('M') && (gcode.getValue('M') == "104" || gcode.getValue('M') == "105" || gcode.getValue('M') == "106" || gcode.getValue('M') == "107" || gcode.getValue('M') == "109"))
			
						// Get next line
						continue;
					
					// Otherwise check if on first counted layer
					else if(layerCounter == 1) {
			
						// Check if wave bonding isn't being used and line is a G command
						if(!overrideWaveBondingPreprocessor && !useWaveBonding && gcode.hasValue('G')) {
						
							// Check if command is G0 or G1 and it's in absolute
							if((gcode.getValue('G') == "0" || gcode.getValue('G') == "1") && !relativeMode) {
					
								// Check if previous command exists and filament is ABS, HIPS, or PLA
								if(!previousGcode.isEmpty() && (filamentType == ABS || filamentType == HIPS || filamentType == PLA)) {
						
									// Check if corner counter is less than or equal to one
									if(cornerCounter <= 1) {
							
										// Check if sharp corner
										if(isSharpCorner(gcode, previousGcode)) {
								
											// Check if refrence g-codes isn't set
											if(refrenceGcode.isEmpty()) {
									
												// Check if a tack point was created
												tackPoint = createTackPoint(gcode, previousGcode);
												if(!tackPoint.isEmpty())
										
													// Send tack point to output
													output << tackPoint << endl;
											}
									
											// Set refrence g-code
											refrenceGcode = gcode;
									
											// Increment corner count
											cornerCounter++;
										}
									}
							
									// Otherwise check if corner counter is greater than one and sharp corner
									else if(cornerCounter >= 1 && isSharpCorner(gcode, refrenceGcode)) {
							
										// Check if a tack point was created
										tackPoint = createTackPoint(gcode, refrenceGcode);
										if(!tackPoint.isEmpty())
								
											// Send tack point to output
											output << tackPoint << endl;
								
										// Set refrence g-code
										refrenceGcode = gcode;
									}
								}
							}
					
							// Otherwise check if command is G90
							else if(gcode.getValue('G') == "90")
				
								// Clear relative mode
								relativeMode = false;
				
							// Otherwise check if command is G91
							else if(gcode.getValue('G') == "91")
				
								// Set relative mode
								relativeMode = true;
						}
					
						// Set previous g-code
						previousGcode = gcode;
					}
				}
				
				// Send line to output file
				output << gcode << endl;
			}
		
			// Return if input file was successfully removed
			return !unlink(tempName.c_str());
		}
	}
	
	// Return false
	return false;
}

bool Printer::bedCompensationPreprocessor(const char *file) {

	// Initialize temporary name
	string tempName = tmpnam(NULL);
	
	// Check if moving the input file to a temporary file was successful
	if(!rename(file, tempName.c_str())) {
		
		// Initialize files
		ifstream input(tempName, ios::in | ios::binary);
		ofstream output(file, ios::out | ios::binary);
	
		// Check if input and output files were opened successfully
		if(input.good() && output.good()) {
		
			// Initialize variables
			string line;
			Gcode gcode, extraGcode;
			bool relativeMode = false;
			bool changesPlane = false;
			double distance, heightAdjustment;
			uint32_t segmentCounter;
			double positionAbsoluteX = 0, positionAbsoluteY = 0;
			double positionRelativeX = 0, positionRelativeY = 0, positionRelativeZ = 0, positionRelativeE = 0;
			double deltaX, deltaY, deltaZ, deltaE;
			double absoluteDifferenceX, absoluteDifferenceY, relativeDifferenceX, relativeDifferenceY, relativeDifferenceZ, relativeDifferenceE;
			double deltaRatioX, deltaRatioY, deltaRatioZ, deltaRatioE;
			double tempAbsoluteX, tempAbsoluteY, tempRelativeX, tempRelativeY, tempRelativeZ, tempRelativeE;
			
			// Go through input file
			while(input.peek() != EOF) {
			
				// Read in a line
				getline(input, line);
				
				// Check if line was parsed successfully and it's a G command
				if(gcode.parseLine(line) && gcode.hasValue('G')) {
			
					// Check if command is G0 or G1 and it's in absolute mode
					if((gcode.getValue('G') == "0" || gcode.getValue('G') == "1") && !relativeMode) {
		
						// Check if command has an X or Y value
						if(gcode.hasValue('X') || gcode.hasValue('Y'))
				
							// Set changes plane
							changesPlane = true;
				
						// Check if command contains a Z value
						if(gcode.hasValue('Z'))
				
							// Add to command's Z value
							gcode.setValue('Z', to_string(stod(gcode.getValue('Z')) + bedHeightOffset));
				
						// Set delta values
						deltaX = !gcode.hasValue('X') ? 0 : stod(gcode.getValue('X')) - positionRelativeX;
						deltaY = !gcode.hasValue('Y')? 0 : stod(gcode.getValue('Y')) - positionRelativeY;
						deltaZ = !gcode.hasValue('Z') ? 0 : stod(gcode.getValue('Z')) - positionRelativeZ;
						deltaE = !gcode.hasValue('E') ? 0 : stod(gcode.getValue('E')) - positionRelativeE;
				
						// Adjust position absolute and relative values for the changes
						positionAbsoluteX += deltaX;
						positionAbsoluteY += deltaY;
						positionRelativeX += deltaX;
						positionRelativeY += deltaY;
						positionRelativeZ += deltaZ;
						positionRelativeE += deltaE;
				
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
						if(distance) {
							deltaRatioX = deltaX / distance;
							deltaRatioY = deltaY / distance;
							deltaRatioZ = deltaZ / distance;
							deltaRatioE = deltaE / distance;
						}
						else {
							deltaRatioX = 0;
							deltaRatioY = 0;
							deltaRatioZ = 0;
							deltaRatioE = 0;
						}
				
						// Check if change in E is greater than zero
						if(deltaE > 0) {
				
							// Go through all segments
							for(uint32_t i = 1; i <= segmentCounter; i++) {
				
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
							
									// Check if command has F value and in first element
									if(gcode.hasValue('F') && i == 1)
							
										// Set extra g-code F value
										extraGcode.setValue('F', gcode.getValue('F'));
							
									// Check if plane changed
									if(changesPlane)
							
										// Set extra g-code Z value
										extraGcode.setValue('Z', to_string(positionRelativeZ - deltaZ + tempRelativeZ - relativeDifferenceZ + heightAdjustment));
							
									// Otherwise check if command has a Z value and the change in Z in noticable
									else if(gcode.hasValue('Z') && deltaZ != DBL_EPSILON)
							
										// Set extra g-code Z value
										extraGcode.setValue('Z', to_string(positionRelativeZ - deltaZ + tempRelativeZ - relativeDifferenceZ));
							
									// Set extra g-gode E value
									extraGcode.setValue('E', to_string(positionRelativeE - deltaE + tempRelativeE - relativeDifferenceE));
							
									// Send extra g-code to output
									output << extraGcode << endl;
								}
						
								// Otherwise
								else {
						
									// Check if plane changed
									if(changesPlane) {
							
										// Check if command has a Z value
										if(gcode.hasValue('Z'))
								
											// Add value to command Z value
											gcode.setValue('Z', to_string(stod(gcode.getValue('Z')) + heightAdjustment));
								
										// Otherwise
										else
								
											// Set command Z value
											gcode.setValue('Z', to_string(relativeDifferenceZ + deltaZ + heightAdjustment));
									}
								}
							}
						}
				
						// Otherwise
						else {
				
							// Check if the plane changed
							if(changesPlane) {
							
								// Set height adjustment
								heightAdjustment = getHeightAdjustmentRequired(positionAbsoluteX, positionAbsoluteY);
						
								// Check if command has a Z value
								if(gcode.hasValue('Z'))
						
									// Add value to command Z
									gcode.setValue('Z', to_string(stod(gcode.getValue('Z')) + heightAdjustment));
						
								// Otherwise
								else
						
									// Set command Z
									gcode.setValue('Z', to_string(positionRelativeZ + heightAdjustment));
							}
						}
					}
					
					// Otherwise check if command is G28
					else if(gcode.getValue('G') == "28") {
				
						// Set X and Y to home
						positionRelativeX = positionAbsoluteX = 54;
						positionRelativeY = positionAbsoluteY = 50;
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
			
							// Set position relative values
							if(gcode.hasValue('X'))
								positionRelativeX = stod(gcode.getValue('X'));
							if(gcode.hasValue('Y'))
								positionRelativeY = stod(gcode.getValue('Y'));
							if(gcode.hasValue('Z'))
								positionRelativeZ = stod(gcode.getValue('Z'));
							if(gcode.hasValue('E'))
								positionRelativeE = stod(gcode.getValue('E'));
						}
					}
				}
				
				// Send line to output file
				output << gcode << endl;
			}
		
			// Return if input file was successfully removed
			return !unlink(tempName.c_str());
		}
	}
	
	// Return false
	return false;
}

bool Printer::backlashCompensationPreprocessor(const char *file) {

	// Initialize temporary name
	string tempName = tmpnam(NULL);
	
	// Check if moving the input file to a temporary file was successful
	if(!rename(file, tempName.c_str())) {
		
		// Initialize files
		ifstream input(tempName, ios::in | ios::binary);
		ofstream output(file, ios::out | ios::binary);
	
		// Check if input and output files were opened successfully
		if(input.good() && output.good()) {
		
			// Initialize variables
			string line;
			Gcode gcode, extraGcode;
			bool relativeMode = false;
			string valueF = "1000";
			directions directionX, directionY, previousDirectionX = NEITHER, previousDirectionY = NEITHER;
			double compensationX = 0, compensationY = 0;
			double positionRelativeX = 0, positionRelativeY = 0, positionRelativeZ = 0, positionRelativeE = 0;
			double deltaX, deltaY, deltaZ, deltaE;
	
			// Go through input file
			while(input.peek() != EOF) {
			
				// Read in a line
				getline(input, line);
				
				// Check if line was parsed successfully and it's a G command
				if(gcode.parseLine(line) && gcode.hasValue('G')) {
				
					// Check if command is G0 or G1 and it's in absolute mode
					if((gcode.getValue('G') == "0" || gcode.getValue('G') == "1") && !relativeMode) {
			
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
							if(directionX != previousDirectionX && previousDirectionX != NEITHER)
					
								// Set X compensation
								compensationX += backlashX * (directionX == POSITIVE ? 1 : -1);
					
							// Check if Y direction has changed
							if(directionY != previousDirectionY && previousDirectionY != NEITHER)
					
								// Set Y compensation
								compensationY += backlashY * (directionY == POSITIVE ? 1 : -1);
							
							// Set extra g-code X and Y value
							extraGcode.setValue('X', to_string(positionRelativeX + compensationX));
							extraGcode.setValue('Y', to_string(positionRelativeY + compensationY));
					
							// Set extra g-code F value
							extraGcode.setValue('F', to_string(backlashSpeed));
					
							// Send extra g-code to output
							output << extraGcode << endl;
					
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
					
					// Otherwise check if command is G28
					else if(gcode.getValue('G') == "28") {
				
						// Set relative values
						positionRelativeX = 54;
						positionRelativeY = 50;
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
			
							// Set position relative values
							if(gcode.hasValue('X'))
								positionRelativeX = stod(gcode.getValue('X'));
							if(gcode.hasValue('Y'))
								positionRelativeY = stod(gcode.getValue('Y'));
							if(gcode.hasValue('Z'))
								positionRelativeZ = stod(gcode.getValue('Z'));
							if(gcode.hasValue('E'))
								positionRelativeE = stod(gcode.getValue('E'));
						}
					}
				}
				
				// Send line to output file
				output << gcode << endl;
			}
		
			// Return if input file was successfully removed
			return !unlink(tempName.c_str());
		}
	}
	
	// Return false
	return false;
}

bool Printer::feedRateConversionPreprocessor(const char *file) {

	// Initialize temporary name
	string tempName = tmpnam(NULL);
	
	// Check if moving the input file to a temporary file was successful
	if(!rename(file, tempName.c_str())) {
		
		// Initialize files
		ifstream input(tempName, ios::in | ios::binary);
		ofstream output(file, ios::out | ios::binary);
	
		// Check if input and output files were opened successfully
		if(input.good() && output.good()) {
		
			// Initialize variables
			string line;
			Gcode gcode;
			double commandFeedRate;
	
			// Go through input file
			while(input.peek() != EOF) {
			
				// Read in a line
				getline(input, line);
				
				// Check if line was parsed successfully and it contains G and F values
				if(gcode.parseLine(line) && gcode.hasValue('G') && gcode.hasValue('F')) {
			
					// Get command's feedrate
					commandFeedRate = stod(gcode.getValue('F')) / 60;
				
					// Force feed rate to adhere to limitations
					if(commandFeedRate > MAX_FEED_RATE)
		        			commandFeedRate = MAX_FEED_RATE;
		        		
					// Set new feed rate for the command
					gcode.setValue('F', to_string(30 + (1 - commandFeedRate / MAX_FEED_RATE) * 800));
				}
				
				// Send line to output file
				output << gcode << endl;
			}
		
			// Return if input file was successfully removed
			return !unlink(tempName.c_str());
		}
	}
	
	// Return false
	return false;
}
