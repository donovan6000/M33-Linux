// Header gaurd
#ifndef PRINTER_H
#define PRINTER_H


// Header files
#include <string>
#include "gcode.h"
#include "vector.h"

using namespace std;


// Definitions

// Printer bed size limits
#define BED_LOW_MAX_X 113.0
#define BED_LOW_MIN_X 0.0
#define BED_LOW_MAX_Y 107.0
#define BED_LOW_MIN_Y 0.0
#define BED_LOW_MAX_Z 5.0
#define BED_LOW_MIN_Z 0.0
#define BED_MEDIUM_MAX_X 110.2
#define BED_MEDIUM_MIN_X 2.8
#define BED_MEDIUM_MAX_Y 107.0
#define BED_MEDIUM_MIN_Y -6.6
#define BED_MEDIUM_MAX_Z 73.5
#define BED_MEDIUM_MIN_Z BED_LOW_MAX_Z
#define BED_HIGH_MAX_X 82.0
#define BED_HIGH_MIN_X 2.35
#define BED_HIGH_MAX_Y 92.95
#define BED_HIGH_MIN_Y 20.05
#define BED_HIGH_MAX_Z 112.0
#define BED_HIGH_MIN_Z BED_MEDIUM_MAX_Z

// Filament details
enum filamentTypes {NO_TYPE, ABS, PLA, HIPS, OTHER};

enum filamentLocations {NO_LOCATION, INTERNAL, EXTERNAL};

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
		Name: Is Bootloader Mode
		Purpose: Return is the printer is in bootloader mode
		*/
		bool isBootloaderMode();
		
		/*
		Name: Get Firnware Version
		Purpose: Returns the printer's firmware version
		*/
		string getFirmwareVersion();
		
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
		Name: Is Z Valid
		Purpose: Checks if the printer's Z is valid
		*/
		bool isZValid();
		
		/*
		Name: Calibrate Z
		Purpose: Calibraters the printer's Z
		*/
		void calibrateZ();
		
		/*
		Name: Is Bed Orientation Valid
		Purpose: Checks if the printer's bed orientation is valid
		*/
		bool isBedOrientationValid();
		
		/*
		Name: Calibrate orientation
		Purpose: Calibraters the printer's bed orientation
		*/
		void calibrateBedOrientation();
		
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
		bool sendRequest(const Gcode &data);
		
		/*
		Name: Receive Response
		Purpose: Receives data to the printer
		*/
		string receiveResponse();
		
		/*
		Name: Process File
		Purpose: Processes a g-code file with the set pre-processor parameters
		*/
		bool processFile(const char *inputFile, const char *outputFile = NULL);
		
		/*
		Name: Print File
		Purpose: Pre-processed and send a file to the printer
		*/
		bool printFile(const char *file);
		
		/*
		Name: Set Pre-processors
		Purpose: Sets which pre-processor stages to use
		*/
		void setValidationPreprocessor();
		void setPreparationPreprocessor();
		void setWaveBondingPreprocessor();
		void setThermalBondingPreprocessor();
		void setBedCompensationPreprocessor();
		void setBacklashCompensationPreprocessor();
		void setFeedRateConversionPreprocessor();
		void setCenterModelPreprocessor();
		
		
		/*
		Name: Set printer values
		Purpose: Sets the printer settings for use in the pre-processor stages
		*/
		void setBacklashX(const string &value);
		void setBacklashY(const string &value);
		void setBacklashSpeed(const string &value);
		void setBedHeightOffset(const string &value);
		void setBackRightOffset(const string &value);
		void setBackLeftOffset(const string &value);
		void setFrontLeftOffset(const string &value);
		void setFrontRightOffset(const string &value);
		void setFilamentTemperature(const string &value);
		void setFilamentType(const string &value);
		
		/*
		Name: Translator Mode
		Purpose: Acts as a translator for other software to communicate with the printer
		*/
		void translatorMode();
		
		/*
		Name: Use settings file
		Purpose: Reads in printer values from settings file
		*/
		bool useSettingsFile();
	
	// Private
	private:
	
		// Write to eeprom
		bool writeToEeprom(uint16_t address, const uint8_t *data, uint16_t length);
		bool writeToEeprom(uint16_t address, uint8_t data);
	
		// Send data to the printer
		bool sendRequestAscii(char data);
		bool sendRequestAscii(const char *data);
		bool sendRequestAscii(const Gcode &data);
		bool sendRequestBinary(const char *data);
		bool sendRequestBinary(const Gcode &data);
		
		// Receive data from the printer
		string receiveResponseAscii();
		string receiveResponseBinary();
		
		// CRC32
		uint32_t crc32(int32_t offset, const uint8_t *data, int32_t count);
		
		// Create settings file
		bool createSettingsFile();
		
		// Checks if file to print doesn't go past the printer's boundaries
		bool checkPrintDimensions(const char *file, bool overrideCenterModelPreprocessor);
		
		// Get max
		double max(double first, double second);
		
		// Get bounded temperature
		uint16_t getBoundedTemperature(uint16_t temperature);
		
		// Get distance
		double getDistance(const Gcode &firstPoint, const Gcode &secondPoint);
		
		// Create tack point
		Gcode createTackPoint(const Gcode &point, const Gcode &refrence);
		
		// Is sharp corner
		bool isSharpCorner(const Gcode &point, const Gcode &refrence);
		
		// get current adjustment Z
		double getCurrentAdjustmentZ();
		
		// Calculate plane normal vector
		Vector calculatePlaneNormalVector(const Vector &v1, const Vector &v2, const Vector &v3);
		
		// Generate plane equation
		Vector generatePlaneEquation(const Vector &v1, const Vector &v2, const Vector &v3);
		
		// Get height adjustment required
		double getHeightAdjustmentRequired(double x, double y);
		
		// Get Z from XY plane
		double getZFromXYAndPlane(const Vector &point, const Vector &planeABC);
		
		// Is point in triangle
		bool isPointInTriangle(const Vector &pt, const Vector &v1, const Vector &v2, const Vector &v3);
		
		// Sign
		double sign(const Vector &p1, const Vector &p2, const Vector &p3);
		
		// Pre-processor stages
		bool validationPreprocessor(const char *file);
		bool preparationPreprocessor(const char *file, bool overrideCornerExcess);
		bool waveBondingPreprocessor(const char *file);
		bool thermalBondingPreprocessor(const char *file, bool overrideWaveBondingPreprocessor);
		bool bedCompensationPreprocessor(const char *file);
		bool backlashCompensationPreprocessor(const char *file);
		bool feedRateConversionPreprocessor(const char *file);
		bool centerModelPreprocessor(const char *file);
		
		// Use pre-processor stages 
		bool useValidation;
		bool usePreparation;
		bool useWaveBonding;
		bool useThermalBonding;
		bool useBedCompensation;
		bool useBacklashCompensation;
		bool useFeedRateConversion;
		bool useCenterModel;
		bool ignorePrintDimensionLimitations;
		
		// Print values
		double maxXExtruderLow;
		double maxXExtruderMedium;
		double maxXExtruderHigh;
		double maxYExtruderLow;
		double maxYExtruderMedium;
		double maxYExtruderHigh;
		double maxZExtruder;
		double minXExtruderLow;
		double minXExtruderMedium;
		double minXExtruderHigh;
		double minYExtruderLow;
		double minYExtruderMedium;
		double minYExtruderHigh;
		double minZExtruder;
		
		// Working folder location
		string workingFolderLocation;
	
		// Bootloader mode
		bool bootloaderMode;
		
		// Valid Z
		bool validZ;
		
		// Valid bed orientation
		bool validBedOrientation;
		
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
		uint16_t backlashSpeed;
		
		// Bed orientations
		double backRightOrientation;
		double backLeftOrientation;
		double frontRightOrientation;
		double frontLeftOrientation;
		
		// Status
		uint8_t status;
		
		// File descriptor
		int fd;
		
		// Virtual port descriptor
		int vd;
		
		// Virtual serial port location
		string virtualSerialPortLocation;
};


#endif
