/*
Transmitter
1.speed control done in app or
with potentiometer connected to pins ----> A0
3.nrf24l01+la+pa radio module connected to pins ---->
sck=13,miso=12,mosi=11,csn=10,ce=9
4.joystick connected to pins -----> x=A1,y=A2
5.hc-05 bluetooth module connected to pins ---> tx=0,rx=1
6.adxl sensor connected to pins -----> sda=sda(second),scl=scl(first)
*/
//radio
#include <SPI.h>

#include <nRF24L01.h>

#include <RF24.h>

RF24 radio(9, 10); // CE, CSN
struct instruction {
    int cspeed = 0;
    int data = 0;
}
message;
const byte address[6] = "00001";
//adxl
#include <Wire.h>

int ADXL345 = 0x53;
float X_out, Y_out, Z_out;
float roll, pitch, rollF, pitchF = 0;
//bluetooth
char bluetooth_data;
char prev_data;
//speed
#define pot A3
//radio
#define ce 9
#define csn 10
//joystick
#define jx
A0
#define jy A1
int x, y = 0;
//calling for adxl 345 sesnsor data
void adxl() {
    //adxl initiation
    Serial.println("adxl sensor");
    Wire.beginTransmission(ADXL345);
    Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is
    stored in 2 registers
    X_out = (Wire.read() | Wire.read() << 8); // X-axis valueX_out = X_out / 256; //For a range of +-2g, we need to divide the raw values by 256,
    according to the datasheet
    Y_out = (Wire.read() | Wire.read() << 8); // Y-axis value
    Y_out = Y_out / 256;
    Z_out = (Wire.read() | Wire.read() << 8); // Z-axis value
    Z_out = Z_out / 256;
    // Calculate Roll and Pitch (rotation around X-axis, rotation around Y-axis)
    roll = atan(Y_out / sqrt(pow(X_out, 2) + pow(Z_out, 2))) * 180 / PI;
    pitch = atan(-1 * X_out / sqrt(pow(Y_out, 2) + pow(Z_out, 2))) * 180 / PI;
    // Low-pass filter
    rollF = 0.94 * rollF + 0.06 * roll;
    pitchF = 0.94 * pitchF + 0.06 * pitch;
    Serial.print(rollF);
    Serial.print("/");
    Serial.println(pitchF);
    Serial.println("adxl mode");
    if (pitchF <= -35) {
        message.data = 1; //forward
    } else {
        if (pitchF >= 35) {
            message.data = 2; //backward
        } else {
            if (rollF >= 35) {
                message.data = 3; //left
            } else {
                if (rollF <= -35) {
                    message.data = 4; //right
                } else {
                    message.data = 5; //halt
                }
            }
        }
    }
}
//calling for joystick module data
void joystick() {
    Serial.println("joystick");
    x = analogRead(jx);
    y = analogRead(jy);
    if (y <= 100) {
        message.data = 1; //forward
    } else {
        if (y >= 900) {
            message.data = 2; //backward
        } else {
            if (x <= 100) {
                message.data = 3; //left
            } else {
                if (x >= 900) {
                    message.data = 4; //right
                } else {
                    message.data = 5; //halt
                }
            }
        }
    }
}
//calling for potentiometer data
void potspeed() {
    message.cspeed = analogRead(pot);
    message.cspeed = map(message.cspeed, 0, 1023, 0, 255);
}
//calling for hc - 05 instructions
void bluetooth() {
    if (Serial.available() > 0) {
        bluetooth_data = Serial.read();
        switch (bluetooth_data) {
        case 'F':
            message.data = 1;
            break;
        case 'B':
            message.data = 2;
            break;
        case 'L':
            message.data = 3;
            break;
        case 'R':
            message.data = 4;
            break;
        case 'H':
            message.data = 5;
            break;
        case 'A':
            message.data = 6; //autonomous mode
            break;
        case 'X':
            message.data = 7; //black line followerbreak;
        case 'W':
            message.data = 8; //white line follower
            break;
        case 'G':
            //guesture control
            adxl();
            break;
        case 'J':
            //joystick control
            joystick();
            break;
        }
    } else {
        switch (bluetooth_data) {
        case 'F':
            message.data = 1;
            break;
        case 'B':
            message.data = 2;
            break;
        case 'L':
            message.data = 3;
            break;
        case 'R':
            message.data = 4;
            break;
        case 'H':
            message.data = 5;
            break;
        case 'A':
            message.data = 6; //autonomous mode
            break;
        case 'X':
            message.data = 7; //black line follower
            break;
        case 'W':
            message.data = 8; //white line follower
            break;
        case 'G':
            //guesture control
            adxl();
            break;
        }
    }
}
//setup
void setup() {
    Serial.begin(9600); // Initiate serial communication
    Wire.begin(); // Initiate the Wire library
    // Set ADXL345 in measuring mode
    Wire.beginTransmission(ADXL345); // Start communicating with the deviceWire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
    // Enable measurement
    Wire.write(8); // Bit D3 High for measuring enable (8dec -> 0000 1000 binary)
    Wire.endTransmission();
    delay(10);
    //Off-set Calibration
    //X-axis
    Wire.beginTransmission(ADXL345);
    Wire.write(0x1E);
    Wire.write(1);
    Wire.endTransmission();
    delay(10);
    //Y-axis
    Wire.beginTransmission(ADXL345);
    Wire.write(0x1F);
    Wire.write(-2);
    Wire.endTransmission();
    delay(10);
    //Z-axis
    Wire.beginTransmission(ADXL345);
    Wire.write(0x20);
    Wire.write(-9);
    Wire.endTransmission();
    delay(10);
    // radio control setup
    radio.begin();
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MIN);
    radio.stopListening();
}
//loop
void loop() {
    //if bluetooth data is available we use it else the default control mode is with
    joystick
    joystick();
    bluetooth();
    potspeed();
    //sending data through radio module
    radio.write( & message, sizeof(instruction));
    Serial.println("--------------data----------------");
    Serial.print("direction:");
    Serial.println(message.data);
    Serial.print("speed:");
    Serial.println(message.cspeed);
    radio.write( & message, sizeof(instruction));
}
