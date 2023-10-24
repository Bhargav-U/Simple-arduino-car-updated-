//receiver
/*
-----ir sensor connections---------------
*5,6- left ir set
*8,7 - right ir set
*4- back ir set
--------ultrasonic sensor connections-----
*
a0,a1 - trig ,echo of ultrasonic sensor
-----radio module connections ------------
*13 - sck
*11 - mosi
*12 - miso
*9 - ce
*10 - csn
-------l288n motor driver connections-----
*a1,a2 - (ls,rs)left and right motor speed control
*0,1,2,3 - lf,lb,rf,rb
*/
#include <SPI.h>

#include <nRF24L01.h>

#include <RF24.h>

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";
struct instruction {
    int cspeed = 0;
    int data = 0;
}
message;
#define front_left_1_ir 5
#define front_left_2_ir 6
#define front_right_1_ir 7
#define front_right_2_ir 8
#define lf 0
#define lb 1
#define rf 2
#define rb 3
#define ls A0
#define rs A1
int motor_speed = 0, front_left_1 = 0, front_left_2 = 0, front_right_1 = 0,
    front_right_2 = 0, back = 0;
void setspeed() {
    motor_speed = message.cspeed;
    digitalWrite(ls, motor_speed);
    digitalWrite(rs, motor_speed);
}
void forward() {
    setspeed();
    digitalWrite(lf, HIGH);
    digitalWrite(rf, HIGH);
    digitalWrite(lb, LOW);
    digitalWrite(rb, LOW);
}
void backward() {
    setspeed();
    digitalWrite(lb, HIGH);
    digitalWrite(rb, HIGH);
    digitalWrite(lf, LOW);
    digitalWrite(rf, LOW);
}
void left() {
    setspeed();
    digitalWrite(lb, HIGH);
    digitalWrite(rf, HIGH);
    digitalWrite(lf, LOW);
    digitalWrite(rb, LOW);
}
void right() {
    setspeed();
    digitalWrite(lf, HIGH);
    digitalWrite(rb, HIGH);
    digitalWrite(lb, LOW);
    digitalWrite(rf, LOW);
}
void halt() {
    setspeed();
    digitalWrite(lf, LOW);
    digitalWrite(rf, LOW);
    digitalWrite(lb, LOW);
    digitalWrite(rb, LOW);
}
void black_line() {}
void white_line() {}
void autonomus() {}
void control() {
    setspeed();
    switch (message.data) {
    case 1:
        forward();
        Serial.println("forward");
        break;
    case 2:
        backward();
        Serial.println("backwwrd");
        break;
    case 3:
        left();
        Serial.println("left");
        break;
    case 4:
        right();
        Serial.println("right");
        break;
    case 5:
        halt();
        Serial.println("halt");
        break;
    case 6:
        autonomus();
        break;
    case 7:
        black_line();
        break;
    case 8:
        white_line();
        break;
    }
}
void setup() {
    Serial.begin(9600);
    pinMode(lf, OUTPUT);
    pinMode(lb, OUTPUT);
    pinMode(rf, OUTPUT);
    pinMode(rb, OUTPUT);
    // radio control setup
    radio.begin();
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
}
void loop() {
    if (radio.available()) {
        radio.read( & message, sizeof(instruction));
        control();
    }
}
