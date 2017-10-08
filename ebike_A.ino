/*
  Ebike lighting control
  using arduino nano clone 
 */
#include "Light.h"
 
Light tail; 
Light head; 

//modes
const int OFF = 0;
const int ON = 1;
const int FLASHING = 2;

//button control constants
const int DEBOUNCE_DELAY = 15;
const int HOLDDOWN_DELAY = 250;

//LED fading constants
const int FADE_INCREMENT = 1;  //must be a divosor of 255, ex 1,3,5.. 
const int PWM_MAX = 255; 
const int PWM_MIN  = 0;
const int PWM_LOW = 100;  

//LED flashing
const int FLASH_TIME = 500;


void setup() {
  //intitialize lights:
  tail = {2,LOW,10,OFF,PWM_MAX,FLASH_TIME,0};   //button pin #2, button state is low, LED pin 10, mode OFF (0) , fade level 255, last flash set to 0
  head = {3,LOW,11,OFF,PWM_MAX,FLASH_TIME,0};   //button pin #3, button state is low, LED pin 11, mode OFF (0), fade level 255, last flash set to 0

  pinMode(tail.buttonPin, INPUT);
  pinMode(head.buttonPin, INPUT);
  pinMode(tail.lightPin, OUTPUT);
  pinMode(head.lightPin, OUTPUT);
  
  Serial.begin(9600);
}

void loop() {
  updateLightMode(tail);
  updateLightMode(head);
  unsigned long currentMillis = millis();
  triggerEffect(tail,currentMillis);
  triggerEffect(head,currentMillis);
}


void triggerEffect(Light &light, unsigned long thisMillis){
  if (light.mode == FLASHING){
    if(thisMillis - light.lastFlash >= light.flashInterval) {
      if (light.lightFade == PWM_MAX){

        light.lightFade = PWM_LOW;
      } else {
        light.lightFade = PWM_MAX;
      }

    analogWrite(light.lightPin, light.lightFade);
    light.lastFlash = thisMillis;
    }
  }
}

void updateLightMode(Light &light) {
  if(buttonPressedCheck(light)) {                 //if a button press has occured
      light.buttonState = HIGH;                //update the recorded button state to high
      delay(HOLDDOWN_DELAY);                  //wait to see if it's a hold down or a press
      if(buttonHoldCheck(light)){                       //if the button is being held down then start adjustingMode
        adjustMode(light);  
      }else {                          //if the button is not being held down  that means it was a press, cycle the mode and write the new lighting condition
        cycleMode(light);   
        writeMode(light);
      }
  } else if(buttonReleasedCheck(light)) {   //if a button release has occured then save the state of the button as not pressed
    light.buttonState = LOW;
  }
}

void adjustMode(Light &light){
  if (light.mode == ON){
    fade(light);
  }else{
    modifyFlashInterval(light);
  }
}

void modifyFlashInterval(Light &light){
  //Serial.println("tried to adjust flashing speed");
  int increment= FADE_INCREMENT;
  

  while(debounceButton(light)){
    unsigned long currentMillis = millis();
    triggerEffect(light,currentMillis);
    light.flashInterval +=increment;
    increment *=(((light.flashInterval % 1000) == PWM_MIN) ? -1 : 1);
    //Serial.println(light.flashInterval);
    delay(5);
  }
  
}



void fade(Light &light) {    //resets light.Fade to 0 and then fades up and down from 0 to 255
  int dir = FADE_INCREMENT;
  light.lightFade = LOW;

  while(debounceButton(light) && light.mode == ON){
    light.lightFade += dir;
    dir *= (((light.lightFade % PWM_MAX) == PWM_MIN) ? -1 : 1);   //cycling math
    analogWrite(light.lightPin, light.lightFade);
    Serial.println(light.lightFade);
  }
}

void cycleMode(Light &light) {
  resetDefaults(light);
  if (light.mode == FLASHING) {
    light.mode = OFF;
  } else {
    light.mode++;
  }
}

void resetDefaults(Light &light){
  light.lightFade=PWM_MAX;
  light.flashInterval = FLASH_TIME;
}


void writeMode(Light light) {
  if(light.mode == OFF) {
    Serial.println("Light mode is OFF");
    analogWrite(light.lightPin, LOW);
  } else if(light.mode == ON) { //second mode of lighting both on
    
    Serial.println("Light mode is ON");
    analogWrite(light.lightPin, light.lightFade);
  } else {
    Serial.println("Light mode is FLASHING");
    analogWrite(light.lightPin, PWM_MAX);
  }
}

boolean buttonPressedCheck(Light light) {
  return (debounceButton(light) == HIGH && light.buttonState == LOW);
}

boolean buttonReleasedCheck(Light light) {
  return (debounceButton(light) == LOW && light.buttonState == HIGH);
}

boolean buttonHoldCheck(Light light) {
  return (debounceButton(light) == HIGH && light.buttonState == HIGH);
}

boolean debounceButton(Light light){
  boolean stateNow = digitalRead(light.buttonPin);
  if (light.buttonState != stateNow) {
    delay(DEBOUNCE_DELAY);
    stateNow = digitalRead(light.buttonPin);
  }
  return stateNow;
}


