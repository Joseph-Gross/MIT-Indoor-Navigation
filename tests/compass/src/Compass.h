// Import statements

#ifndef INC_6_08_PATH_FINDING_COMPASS_H
#define INC_6_08_PATH_FINDING_COMPASS_H

#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library NEEDED FOR COMPASS DISPLA/\Y
#include <Adafruit_ST7735.h> // Hardware-specific library NEEDED FOR COMPASS SENSING
#include <math.h>
#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// #define BACKGROUND TFT_BLACK // Already defined?
#define pi 3.14159265
#define degree_to_rad 0.0174532925; // pi/180.0
#define root3over2 0.8660254
#define root3 1.7320508

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
#define Serial SERIAL_PORT_USBVIRTUAL                               // Required for Serial on Zero based boards
#endif

#define LCD2
#define BACKGROUND TFT_BLACK
#define True_North true          // change this to "true" for True North  

// ----- MPU-9250 addresses
#define MPU9250_ADDRESS     0x68  // Device address when ADO = 0; Use 0x69 when AD0 = 1
#define AK8963_ADDRESS      0x0C  //  Address of magnetometer
// ----- MPU-9250 register map
#define AK8963_WHO_AM_I     0x00  // should return 0x48
#define AK8963_INFO         0x01
#define AK8963_ST1          0x02  // data ready status bit 0
#define AK8963_XOUT_L       0x03  // data
#define AK8963_XOUT_H       0x04
#define AK8963_YOUT_L       0x05
#define AK8963_YOUT_H       0x06
#define AK8963_ZOUT_L       0x07
#define AK8963_ZOUT_H       0x08
#define AK8963_ST2          0x09  // Data overflow bit 3 and data read error status bit 2
#define AK8963_CNTL         0x0A  // Power down (0000), single-measurement (0001), self-test (1000) and Fuse ROM (1111) modes on bits 3:0
#define AK8963_ASTC         0x0C  // Self test control
#define AK8963_I2CDIS       0x0F  // I2C disable
#define AK8963_ASAX         0x10  // Fuse ROM x-axis sensitivity adjustment value
#define AK8963_ASAY         0x11  // Fuse ROM y-axis sensitivity adjustment value
#define AK8963_ASAZ         0x12  // Fuse ROM z-axis sensitivity adjustment value

#define SELF_TEST_X_GYRO    0x00
#define SELF_TEST_Y_GYRO    0x01
#define SELF_TEST_Z_GYRO    0x02

/*
  #define X_FINE_GAIN         0x03   // [7:0] fine gain
  #define Y_FINE_GAIN         0x04
  #define Z_FINE_GAIN         0x05
  #define XA_OFFSET_H         0x06   // User-defined trim values for accelerometer
  #define XA_OFFSET_L_TC      0x07
  #define YA_OFFSET_H         0x08
  #define YA_OFFSET_L_TC      0x09
  #define ZA_OFFSET_H         0x0A
  #define ZA_OFFSET_L_TC      0x0B
*/

#define SELF_TEST_X_ACCEL   0x0D
#define SELF_TEST_Y_ACCEL   0x0E
#define SELF_TEST_Z_ACCEL   0x0F

#define SELF_TEST_A         0x10

#define XG_OFFSET_H         0x13   // User-defined trim values for gyroscope
#define XG_OFFSET_L         0x14
#define YG_OFFSET_H         0x15
#define YG_OFFSET_L         0x16
#define ZG_OFFSET_H         0x17
#define ZG_OFFSET_L         0x18
#define SMPLRT_DIV          0x19
#define CONFIG              0x1A
#define GYRO_CONFIG         0x1B
#define ACCEL_CONFIG        0x1C
#define ACCEL_CONFIG2       0x1D
#define LP_ACCEL_ODR        0x1E
#define WOM_THR             0x1F

#define MOT_DUR             0x20   // Duration counter threshold for motion interrupt generation, 1 kHz rate, LSB = 1 ms
#define ZMOT_THR            0x21   // Zero-motion detection threshold bits [7:0]
#define ZRMOT_DUR           0x22   // Duration counter threshold for zero motion interrupt generation, 16 Hz rate, LSB = 64 ms

#define FIFO_EN             0x23
#define I2C_MST_CTRL        0x24
#define I2C_SLV0_ADDR       0x25
#define I2C_SLV0_REG        0x26
#define I2C_SLV0_CTRL       0x27
#define I2C_SLV1_ADDR       0x28
#define I2C_SLV1_REG        0x29
#define I2C_SLV1_CTRL       0x2A
#define I2C_SLV2_ADDR       0x2B
#define I2C_SLV2_REG        0x2C
#define I2C_SLV2_CTRL       0x2D
#define I2C_SLV3_ADDR       0x2E
#define I2C_SLV3_REG        0x2F
#define I2C_SLV3_CTRL       0x30
#define I2C_SLV4_ADDR       0x31
#define I2C_SLV4_REG        0x32
#define I2C_SLV4_DO         0x33
#define I2C_SLV4_CTRL       0x34
#define I2C_SLV4_DI         0x35
#define I2C_MST_STATUS      0x36
#define INT_PIN_CFG         0x37
#define INT_ENABLE          0x38
#define DMP_INT_STATUS      0x39  // Check DMP interrupt
#define INT_STATUS          0x3A
#define ACCEL_XOUT_H        0x3B
#define ACCEL_XOUT_L        0x3C
#define ACCEL_YOUT_H        0x3D
#define ACCEL_YOUT_L        0x3E
#define ACCEL_ZOUT_H        0x3F
#define ACCEL_ZOUT_L        0x40
#define TEMP_OUT_H          0x41
#define TEMP_OUT_L          0x42
#define GYRO_XOUT_H         0x43
#define GYRO_XOUT_L         0x44
#define GYRO_YOUT_H         0x45
#define GYRO_YOUT_L         0x46
#define GYRO_ZOUT_H         0x47
#define GYRO_ZOUT_L         0x48
#define EXT_SENS_DATA_00    0x49
#define EXT_SENS_DATA_01    0x4A
#define EXT_SENS_DATA_02    0x4B
#define EXT_SENS_DATA_03    0x4C
#define EXT_SENS_DATA_04    0x4D
#define EXT_SENS_DATA_05    0x4E
#define EXT_SENS_DATA_06    0x4F
#define EXT_SENS_DATA_07    0x50
#define EXT_SENS_DATA_08    0x51
#define EXT_SENS_DATA_09    0x52
#define EXT_SENS_DATA_10    0x53
#define EXT_SENS_DATA_11    0x54
#define EXT_SENS_DATA_12    0x55
#define EXT_SENS_DATA_13    0x56
#define EXT_SENS_DATA_14    0x57
#define EXT_SENS_DATA_15    0x58
#define EXT_SENS_DATA_16    0x59
#define EXT_SENS_DATA_17    0x5A
#define EXT_SENS_DATA_18    0x5B
#define EXT_SENS_DATA_19    0x5C
#define EXT_SENS_DATA_20    0x5D
#define EXT_SENS_DATA_21    0x5E
#define EXT_SENS_DATA_22    0x5F
#define EXT_SENS_DATA_23    0x60
#define MOT_DETECT_STATUS   0x61
#define I2C_SLV0_DO         0x63
#define I2C_SLV1_DO         0x64
#define I2C_SLV2_DO         0x65
#define I2C_SLV3_DO         0x66
#define I2C_MST_DELAY_CTRL  0x67
#define SIGNAL_PATH_RESET   0x68
#define MOT_DETECT_CTRL     0x69
#define USER_CTRL           0x6A   // Bit 7 enable DMP, bit 3 reset DMP
#define PWR_MGMT_1          0x6B   // Device defaults to the SLEEP mode
#define PWR_MGMT_2          0x6C
#define DMP_BANK            0x6D   // Activates a specific bank in the DMP
#define DMP_RW_PNT          0x6E   // Set read/write pointer to a specific start address in specified DMP bank
#define DMP_REG             0x6F   // Register in DMP from which to read or to which to write
#define DMP_REG_1           0x70
#define DMP_REG_2           0x71
#define FIFO_COUNTH         0x72
#define FIFO_COUNTL         0x73
#define FIFO_R_W            0x74
#define WHO_AM_I_MPU9250    0x75   // Should return 0x71; MPU9255 will return 0x73
#define XA_OFFSET_H         0x77
#define XA_OFFSET_L         0x78
#define YA_OFFSET_H         0x7A
#define YA_OFFSET_L         0x7B
#define ZA_OFFSET_H         0x7D
#define ZA_OFFSET_L         0x7E

// ----- Mahony free parameters
//#define Kp 2.0f * 5.0f                            // original Kp proportional feedback parameter in Mahony filter and fusion scheme
#define Kp 40.0f                                    // Kp proportional feedback parameter in Mahony filter and fusion scheme
#define Ki 0.0f                                     // Ki integral parameter in Mahony filter and fusion scheme
// ----- user offsets and scale-factors



// ----- Select a TASK
/*
  Choose a TASK from the following list:
  #define TASK 1    // Calibrate once ... using onboard code
  #define TASK 7 // not a real task and nothing will happen hopefully
*/

#define center_x 128/2;

#define TASK 7 

struct Vec {
  float x;
  float y;
};
struct RGB { //looks like this won't actually be used since tft.stroke(r,g,b) isn't a real function I can use
  int r;
  int g;
  int b;
};
// ----- Set initial input parameters
enum Ascale {
  AFS_2G = 0,
  AFS_4G,
  AFS_8G,
  AFS_16G
};

enum Gscale {
  GFS_250DPS = 0,
  GFS_500DPS,
  GFS_1000DPS,
  GFS_2000DPS
};

enum Mscale {
  MFS_14BITS = 0,               // 0.6 mG per LSB;
  MFS_16BITS                    // 0.15 mG per LSB
};

enum M_MODE {
  M_8HZ = 0x02,                 // 8 Hz ODR (output data rate) update
  M_100HZ = 0x06                // 100 Hz continuous magnetometer
};

class Compass {
  private:
    TFT_eSPI* tft;
    Vec center; // for display
    Vec p1, p2, p3, p4, p5, p6, p7; // the points that define the arrow
    float device_angle; //for storing the angle sensed by magnetometer
    int center_y; // only y center should change We don't need to fit that much on the screen
    float Mag_x_offset, Mag_y_offset, Mag_z_offset, Mag_x_scale, Mag_y_scale, Mag_z_scale;
    RGB color;
    int length; // based on where we want to center the arrow we should be able to scale its length so it doesn't go off the screen.
    int width;
    int left_limit; //left side of screen limit
    int right_limit; //right side of screen limit
    int top_limit; //top of screen limit
    int bottom_limit; //bottom of screen limit
    // ----- Magnetic declination
    /*
      The magnetic declination for Lower Hutt, New Zealand is +22.5833 degrees
      Obtain your magnetic declination from http://www.magnetic-declination.com/
      By convention, declination is positive when magnetic north
      is east of true north, and negative when it is to the west.
      Substitute your magnetic declination for the "Declination" shown below.
    */
    float Declination;
    char InputChar;
    bool LinkEstablished;
    String OutputString;
    unsigned long Timer1;
    unsigned long Stop1;
    // ----- Specify sensor full scale
    byte Gscale;
    byte Ascale;
    byte Mscale;                           // Choose either 14-bit or 16-bit magnetometer resolution (AK8963=14-bits)
    byte Mmode;                                  // 2 for 8 Hz, 6 for 100 Hz continuous magnetometer data read
    float aRes, gRes, mRes;                             // scale resolutions per LSB for the sensor
    short accelCount[3];                                // Stores the 16-bit signed accelerometer sensor output
    short gyroCount[3];                                 // Stores the 16-bit signed gyro sensor output
    short magCount[3];                                  // Stores the 16-bit signed magnetometer sensor output
    float magCalibration[3], magBias[3], magScale[3];    // Factory mag calibration, mag offset , mag scale-factor
    float gyroBias[3], accelBias[3];        // Bias corrections for gyro and accelerometer
    short tempCount;                                    // temperature raw count output
    float temperature;                                  // Stores the real internal chip temperature in degrees Celsius
    float SelfTest[6];                                  // holds results of gyro and accelerometer self test
    // ----- global constants for 9 DoF fusion and AHRS (Attitude and Heading Reference System)
    float GyroMeasError;        // gyroscope measurement error in rads/s (start at 40 deg/s)
    float GyroMeasDrift;        // gyroscope measurement drift in rad/s/s (start at 0.0 deg/s/s)
    unsigned long delt_t;                           // used to control display output rate
    unsigned long count, sumCount;              // used to control display output rate
    float pitch, roll, yaw;
    float deltat, sum;                    // integration interval for both filter schemes
    unsigned long lastUpdate, firstUpdate;      // used to calculate integration interval
    unsigned long Now;                              // used to calculate integration interval
    float ax, ay, az, gx, gy, gz, mx, my, mz;           // variables to hold latest sensor data values
    float q[4];              // vector to hold quaternion
    float eInt[3];                 // vector to hold integral error for Mahony method
    void getMres();
    void getGres();
    void getAres();
    void readAccelData(short* destination);
    void readGyroData(short* destination);
    void readMagData(short* destination);
    short readTempData();
    void initAK8963(float* destination);
    void initMPU9250();
    void calibrateMPU9250(float* dest1, float* dest2);
    void magCalMPU9250(float* bias_dest, float* scale_dest);
    void MPU9250SelfTest(float* destination);
    void writeByte(byte address, byte subAddress, byte data);
    byte readByte(byte address, byte subAddress);
    void readBytes(byte address, byte subAddress, byte count, byte* dest);
    void MahonyQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz);
    int angle_return();
  public:
    Compass(TFT_eSPI* _tft, int center_y);
    void update_display(int distance, float dir_next_node);
    void initialize();
    void refresh_data();
    void calc_quaternion();
    void calibrate();
};


#endif //INC_6_08_PATH_FINDING_COMPASS_H
