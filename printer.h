// Header gaurd
#ifndef PRINTER_H
#define PRINTER_H


// Header files
#include <string>

using namespace std;


// Definitions

// Printer bed size limits
#define BED_LOW_MAX_X 112.95
#define BED_LOW_MIN_X 0.05
#define BED_LOW_MAX_Y 106.95
#define BED_LOW_MIN_Y 0.05
#define BED_LOW_MAX_Z 5
#define BED_LOW_MIN_Z 0
#define BED_MEDIUM_MAX_X 110.15
#define BED_MEDIUM_MIN_X 2.85
#define BED_MEDIUM_MAX_Y 106.95
#define BED_MEDIUM_MIN_Y -6.55
#define BED_MEDIUM_MAX_Z 73.5
#define BED_MEDIUM_MIN_Z BED_LOW_MAX_Z
#define BED_HIGH_MAX_X 81.95
#define BED_HIGH_MIN_X 2.4
#define BED_HIGH_MAX_Y 92.89999
#define BED_HIGH_MIN_Y 20.1
#define BED_HIGH_MAX_Z 111.95
#define BED_HIGH_MIN_Z BED_MEDIUM_MAX_Z

// Filament details
enum filamentTypes {UNKNOWN, ABS, PLA, HIPS, OTHER};

enum filamentLocations {INTERNAL, EXTERNAL};

enum filamentColors {BLACK, BLUE, BROWN, GOLD, PINK, GREEN, LIGHT_BLUE, LIGHT_GREEN, MAGENTA, NATURAL, NEON_BLUE, NEON_ORANGE, NEON_YELLOW, ORANGE, PURPLE, RED, SILVER, WHITE, YELLOW = 0x13, OTHER_COLOR, PHANTOM_WHITE, HONEY_CLEAR, FUCHSIA_RED, CHRIMSON_RED, FIRE_ORANGE, MANGO_YELLOW, SHAMROCK_GREEN, COBALT_BLUE, CARRIBEAN_BLUE, MULBERRY_PURPLE = 0x1E, TITANIUM_SILVER, CHARCOAL_BLACK, WHITE_PEARL, CRYSTAL_CLEAR, LIGHT_FUCHSIA, DEEP_CRIMSON, DEEP_SHAMROCK, DEEP_COLBALT, LIGHT_CARRIBEAN = 0x27, DEEP_MULBERRY, SATELLITE_SILVER, DEEP_LEMON, SUNSET_ORANGE, ONYX_BLACK, DRANGON_RED_TOUCH, DRAGON_RED_HOT, ROSE_RED_TOUCH, CORAL_ORANGE_TOUCH, CORAL_ORANGE_WARM, DESPICABLE_YELLOW_TOUCH, MONSTER_GREEN_TOUCH = 0x33, GENIT_BLUE_TOUCH, GENIE_BLUE_ICE, GARGOYLE_BLACK_TOUCH, TRICHROME_NEBULA, TRICHROME_TIGER, FUTURE_ABS = 0x4000, FUTURE_FILAMENT = 0x8000};


// Class
class Printer {

	// Public
	public:
	
		/*
		Name: Constructor
		Purpose: Initializes the variables
		*/
		Printer();
		
		/*
		Name: Destructor
		Purpose: Uninitializes the variables
		*/
		~Printer();
	
		/*
		Name: Connect
		Purpose: Connects or reconnects to the printer
		*/
		bool connect();
		
		/*
		Name: Is Firmware Valid
		Purpose: Checks if the printer's firmware is valid
		*/
		bool isFirmwareValid();
		
		/*
		Name: Update Firmware
		Purpose: Updates the printer's firmware to the provided file
		*/
		bool updateFirmware(const char *file);
		
		/*
		Name: Collect information
		Purpose: Collects information from the printer
		*/
		bool collectInformation();
	
		/*
		Name: Send Request
		Purpose: Sends data to the printer
		*/
		bool sendRequest(const char *data);
		bool sendRequest(const string &data);
	
		/*
		Name: Receive Response
		Purpose: Receives data to the printer
		*/
		string receiveResponse();
	
	// Private
	private:
	
		// Write to eeprom
		bool writeToEeprom(uint16_t address, const uint8_t *data, uint16_t length);
		bool writeToEeprom(uint16_t address, uint8_t data);
	
		// Send data to the printer
		bool sendRequestAscii(char data);
		bool sendRequestAscii(const char *data);
		bool sendRequestBinary(const char *data);
		
		// Receive data from the printer
		string receiveResponseAscii();
		string receiveResponseBinary();
		
		// CRC32
		uint32_t crc32(int32_t offset, const uint8_t *data, int32_t count);
	
		// Bootloader mode
		bool bootloaderMode;
		
		// Valid Z
		bool validZ;
		
		// Valid firmware
		bool validFirmware;
		
		// Firmware version
		string firmwareVersion;
		
		// Serial number
		string serialNumber;
		
		// Filament information
		filamentTypes filamentType;
		filamentLocations filamentLocation;
		filamentColors filamentColor;
		uint16_t filamentTemperature;
		
		// Bed offsets
		double backRightOffset;
		double backLeftOffset;
		double frontRightOffset;
		double frontLeftOffset;
		double bedHeightOffset;
		
		// Backlash settings
		double backlashX;
		double backlashY;
		
		// Bed orientations
		double backRightOrientation;
		double backLeftOrientation;
		double frontRightOrientation;
		double frontLeftOrientation;
		
		// Status
		uint8_t status;
		
		// File descriptor
		int fd;
};


#endif
