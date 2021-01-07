#include <PID_v1.h>
#include <Wire.h>
#include <Arduino.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>

int motorPin = 5;
int potPin = 3;
int interruptPin = 2;
int potVal = 0;
int motorVal = 0;
int ledPin = 12;
int switchPin = 6; //sets the pin that the PID switch is attached to
int switchState = 0; //sets the value of the switchstate to zero

float kP = 1; //sets the kP value to .2
float kI = 1.8; //sets the kI value to .2
float kD = .4; //sets the kD value to .05
float gap; //initializes the variable error
float previousGap;
float Drive; //initializes the variable Drive
float dt = .2; //sets the dt to .2
float Integral; //initializes the variable Integral
float Derivative; //initializes the variable derivative
float Output;
int Setpoint; //initializes the variable setpoint
float Input; //initializes the variable Input
int lastPID; //initializes the varaible last PID


unsigned long previousMillis = 0; //initializes previousMillis as a variable satrting at zero
unsigned long currentMillis = 0; //initializes currentMillis as a variable starting at zero
long REV;
long rpm;
long maxRPM = 6100;
long val;
long num;

volatile byte state = HIGH;

//PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7);

void setup(){
    pinMode(motorPin, OUTPUT);
    pinMode(ledPin, OUTPUT);
    pinMode(potPin, INPUT);
    pinMode(switchPin, INPUT);
    pinMode(interruptPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interruptPin), counter, RISING);
    attachInterrupt(digitalPinToInterrupt(interruptPin), counter, FALLING);  
    Serial.begin(9600);
    lcd.begin(16, 2);
    lcd.setBacklightPin(3 , POSITIVE);
    lcd.setBacklight(HIGH);
    lcd.setCursor(0,0);
    lcd.print("Good day!");
    delay(500);

    Serial.begin(9600);
    lcd.begin (16, 2); // for 16 x 2 LCD module
    lcd.setCursor(0,0);
    lcd.print("Input Speed:");
    //pinMode(buttonPin, INPUT);
    lcd.setCursor(0,1);
    lcd.print("Actual Speed:");
}

void loop(){
    int num = map(potVal, 0, 1023, 0, 100);
    Input = val;
    Serial.println(Input);
    Setpoint = num;

    potVal = analogRead(potPin);
    motorVal = map(potVal, 0, 1023, 0, 255);

    switchState = digitalRead(switchPin);

    currentMillis = millis(); //starts the counting of the currentMillis
    
    if(currentMillis - previousMillis>200){ //every 200 milliseconds
        lcd.setCursor(13,0);
        lcd.print("   ");
        lcd.setCursor(13,1);
        lcd.print("   ");   

        rpm = 30*1000/(millis() - previousMillis)*REV;       //  CALCULATE  RPM USING REVOLUTIONS AND ELAPSED TIME                 
        REV = 0;

        detachInterrupt(interruptPin);

        if(switchState == 1){
        //Serial.println("on"); 
        gap = Setpoint-Input; //distance away from setpoint
        Integral = Integral + (gap*dt); //Adds the integral term to the error
        Derivative = (gap - previousGap)/dt; //calculates the rate of change
        Output = (gap*kP) + (Integral*kI) + (Derivative*kD); //adds all terms up
        previousGap = gap; //sets the previous error to error

        if(Output > 255) //drive cannot exceed 255
            Output = 255;
        if(Output<0)
            Output = 0;
        if(Setpoint>96){
            analogWrite(motorPin, 255);
            Setpoint = 100;
        }else
            analogWrite(motorPin,Output);
    }else{
        //Serial.println("off");
        analogWrite(motorPin, motorVal);
        //rpm = 0;
    }

    digitalWrite(ledPin, state);
    lastPID = millis(); //sets the last PID to be the current time 
    previousMillis = currentMillis; //temporarily sets the previous to the current   
    attachInterrupt(digitalPinToInterrupt(interruptPin), counter, RISING);
    attachInterrupt(digitalPinToInterrupt(interruptPin), counter, FALLING);

    if(rpm > maxRPM)
        maxRPM = rpm;       
    val = rpm;
    val = map(val, 0, maxRPM, 0, 100);
    //Serial.print(gap);
    //Serial.print("\t");
    //Serial.print(Integral);
    //Serial.print("\t");
    // Serial.println(val);

    lcd.setCursor(13,0);
    lcd.print(num);
    lcd.setCursor(13,1);
    lcd.print(val);
    }
}

void counter(){
    REV = REV + 1;          // INCREASE REVOLUTIONS
    if(REV % 15 == 0)
    state = !state; 
}