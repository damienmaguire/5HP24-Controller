
/*
v8 control code for 5hp24
Active low shift paddles for E31

Reads a value from analog 0 quiv to the position of the shifter.
p=584
r=515
n=643
d=324
4=91
3=368
2=626

walks the gearbox up from 1st to 5th in drive
 */
 
#include <Wire.h>
#include <LiquidTWI.h>

LiquidTWI lcd(0); //library for I2C lcd controller
 
const int mv1 = 4; // mv1 on digital 4
const int mv2 = 7;//mv2 on digital 7
const int mv3 = 8; //mv3 on digital 8
const int eds1 = 3; //eds1 line press valve on digital 3 pwm
const int eds2 = 5; // eds2 on digital 5 pwm
const int eds3 = 6; // eds3 on digital 6 pwm
const int eds4  = 9; // eds 4 on digital 9 pwm tcc
const int eds5 = 10; //eds5 on digital 10 pwm

const int upshift  = 11; // upshift paddle
const int dnshift = 12; //  downshift paddle
 
int shifter = 0; //range switch on analog 0
int Temp = 1; //temp sensor on analog 1

int currentgear = 1;
int previousgear = 1;

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

const int numReadings = 10;

int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average


void setup() {
  Serial.begin(9600);
  
 lcd.begin(20, 4); //fire up i2c lcd
 Wire.begin();
  
  pinMode(mv1,OUTPUT); // mv lines as output
  pinMode(mv2,OUTPUT);
  pinMode(mv3,OUTPUT);
  
  pinMode(upshift,INPUT); // paddle switches as inputs
  pinMode(dnshift,INPUT);
  
  digitalWrite(mv1,LOW); //all mv solenoids off at startup
  digitalWrite(mv2,LOW);
  digitalWrite(mv3,LOW);
  
  analogWrite(eds1,0); // all eds valves at zero at startup
  analogWrite(eds2,0);
  analogWrite(eds3,0);
  analogWrite(eds4,0);
  analogWrite(eds5,0);
  
    for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0; 
  
   lcd.clear();
    lcd.setBacklight(HIGH);
 lcd.print("ZF 5HP24 Controller");
  
}

void loop() {
  char oldgear = RangeSW();
   switch(oldgear){
    case 'p': //park
    Park();    
    break;
    case 'r': //reverse
    Reverse();
    break;
    case 'n': //drive
    Neutral();
    break;
    case 'd': //drive
    currentgear = 2; //start in 2nd gear
    Drive();
    break;
    case 'f': //4th
    Four();
    break;
    case 't': //3rd
    Three();
    break;
    case 's': //2nd
    Two();
    break;
  }
 
 
while (RangeSW() == oldgear)
 { 
   delay(100); //wait a sec in the loop
  transtemp(); //read and display gearbox oil temp
  
  if (oldgear == 'd') { //if we are in drive then we need to monitor shift paddles
  previousgear = currentgear; //
  if (UPShift() == 0 && currentgear < 5) ++currentgear; // if we are in a gear lees than 5th and we hit upshift then upshift if not we are already in top gear.
  if (DNShift() == 0 && currentgear > 1) --currentgear; // if we are not in first then we can downshift if down shift paddle is hit
  if (currentgear != previousgear) Drive();               // gearchange detected so we must recall the drive sub to update the gear
  }
  
 }
 
  
} 
  
char RangeSW(){ 
 shifter = analogRead(A0);//read the shifter range switch position 
lcd.setCursor(0, 1);
lcd.print("Selector:");
  if (shifter<600 && shifter>570)
  {
  lcd.print("Park   ");
  return 'p'; //park
  }
  else if (shifter<550 && shifter>500)
  {
    lcd.print("Reverse");
    return 'r'; // reverse
  }
    else if (shifter<660 && shifter>635)
  {
    lcd.print("Neutral");
    return 'n'; //neutral
  }
    else if (shifter<350 && shifter>300)
  {
    lcd.print("Drive  ");
    return 'd'; //drive
  }
    else if (shifter<150 && shifter>50)
  {
    lcd.print("Four   ");
    return 'f'; //fourth
  }
    else if (shifter<400 && shifter>350)
  {
    lcd.print("Three  ");
    return 't'; //third
  }
    else if (shifter<640 && shifter>610)
  {
    lcd.print("Two    ");
    return 's'; //second
  }
  else
  {
     lcd.print("Fault! ");
  }
}

void transtemp(){ //display gearbox oil temp

  // subtract the last reading:
  total= total - readings[index];        
  // read from the sensor:  
  readings[index] = analogRead(Temp);
  // add the reading to the total:
  total= total + readings[index];      
  // advance to the next position in the array:  
  index = index + 1;                    

  // if we're at the end of the array...
  if (index >= numReadings)              
    // ...wrap around to the beginning:
    index = 0;                          

  // calculate the average:
  average = total / numReadings;        
  // send it to the computer as ASCII digits
 // Serial.println(average);  
  delay(1);        // delay in between reads for stability



lcd.setCursor(0, 2);
lcd.print("Oil Temp:");
float oiltemp = ((370-(average))*.75);

lcd.print(oiltemp);
lcd.print("C   ");
}

  
void Park(){ // set valves to park position
    Serial.println("Park");
    analogWrite(eds1,200); //set eds 1 high for low line press
    analogWrite(eds3,125); //set eds 3 at 50% for test
    analogWrite(eds4,0); //set eds 4 low to drop out tcc
    for (int i=0; i <= 255; i++){ //ramp eds 2 from low to high press
    analogWrite(eds2, i);
    delay(1);
    }
    digitalWrite(mv1,HIGH); // set mv1 high
    digitalWrite(mv2,LOW); // mv2 low
    digitalWrite(mv3,HIGH); // mv3 high
    for (int i=0; i <= 255; i++){ //ramp eds 5 from low to high press
    analogWrite(eds5, i);
    delay(1);
   }
}

void Reverse(){ //set valves for reverse
    Serial.println("Reverse");
    analogWrite(eds1,175); //set eds 1 low to bring up line press to 50%
    analogWrite(eds4,0); //set eds 4 low to drop out tcc
    analogWrite(eds3,125); //set eds 3 at 50% for test
    digitalWrite(mv1,LOW); // set mv1 low
    digitalWrite(mv2,HIGH); // mv2 high
    digitalWrite(mv3,LOW); // mv3 low
    for (int i=255; i >= 0; i--){ //ramp eds 2 from high to low press , shift cushion?
    analogWrite(eds2, i);
    delay(1);
    }   
    for (int i=255; i >= 0; i--){ //ramp eds 5 from high to low press , shift cushion?
    analogWrite(eds5, i);
    delay(1);   
    }
}

void Neutral(){ //setup for neutral
    Serial.println("Neutral");
    analogWrite(eds1,200); //set eds 1 high for low line press
    analogWrite(eds3,125); //set eds 3 at 50% for test
    analogWrite(eds4,0); //set eds 4 low to drop out tcc
    for (int i=0; i <= 255; i++){ //ramp eds 2 from low to high press
    analogWrite(eds2, i);
    delay(1);
    }
    digitalWrite(mv1,HIGH); // set mv1 high
    digitalWrite(mv2,LOW); // mv2 low
    digitalWrite(mv3,HIGH); // mv3 high
    for (int i=0; i <= 255; i++){ //ramp eds 5 from low to high press
    analogWrite(eds5, i);
    delay(1);
   }
}
void Drive(){ //setup for drive ............. as a test we walk the box up through the gears with a change every 5 secs
    Serial.println("Drive");
    
    if (currentgear == 1) DOne();
    if (currentgear == 2) DTwo();
    if (currentgear == 3) DThree();
    if (currentgear == 4) DFour();
    if (currentgear == 5) DFive();

   
}

void Four(){
}

void Three(){
}

void Two(){
}

void DOne(){ //drive first gear
    analogWrite(eds1,175); //set eds 1 low to bring up line press
    analogWrite(eds4,0); //set eds 4 low to drop out tcc
    delay(10);
    digitalWrite(mv1,HIGH); // set mv1 high 1st gear for test
    digitalWrite(mv2,LOW); // mv2 low
    digitalWrite(mv3,LOW); // mv3 low
    delay(10);
    analogWrite(eds3,100); //set eds 3 
    delay(10);
    analogWrite(eds2,150);
    analogWrite(eds5,150);
     analogWrite(eds1,25); //set eds 1 low to bring up line press
    lcd.setCursor(0, 3);
    lcd.print("Gear:");
    lcd.print("1st");
    Serial.println("First");
}

void DTwo(){
     analogWrite(eds1,175); //set eds 1 low to bring up line press
     analogWrite(eds4,0); //set eds 4 low to drop out tcc
    delay(10);
    digitalWrite(mv1,HIGH); // set for 2nd
    digitalWrite(mv2,HIGH); // mv2 low
    digitalWrite(mv3,LOW); // mv3 low
    analogWrite(eds2,0); //eds 2 off for second gear
 //   for (int i=150; i >= 0; i--){ //ramp eds 2 from high to low press , shift cushion?
  //  analogWrite(eds2, i);
  //  delay(1);
  //  }
    
    //analogWrite(eds5,250);
    
    for (int i=140; i <= 250; i++){ //ramp eds 5 from low to high press
    analogWrite(eds5, i);
    delay(1);
   }
    
    //analogWrite(eds3,250); //set eds 3 
    
    for (int i=140; i <= 250; i++){ //ramp eds 3 from low to high press
    analogWrite(eds3, i);
    delay(1);
   }
    delay(10);
    analogWrite(eds1,25); //set eds 1 low to bring up line press
   // analogWrite(eds2,0);
   // analogWrite(eds5,50); // second gear
    lcd.setCursor(0, 3);
    lcd.print("Gear:");
    lcd.print("2nd");
    Serial.println("Second");
}

void DThree(){
     analogWrite(eds1,175); //set eds 1 low to bring up line press
     analogWrite(eds4,0); //set eds 4 low to drop out tcc
     analogWrite(eds5,0);
    delay(100);
    digitalWrite(mv1,LOW); // set mv1 for 3rd
    digitalWrite(mv2,HIGH); // mv2 low
    digitalWrite(mv3,LOW); // mv3 low
    analogWrite(eds2,250);
    analogWrite(eds3,250); //set eds 3 
    delay(10);
    analogWrite(eds1,25); //set eds 1 low to bring up line press
   // analogWrite(eds2,0);
   // analogWrite(eds5,0); //third gear
    lcd.setCursor(0, 3);
    lcd.print("Gear:");
    lcd.print("3rd");
    Serial.println("Third");
}

void DFour(){
     analogWrite(eds1,175); //set eds 1 low to bring up line press
     analogWrite(eds4,0); //set eds 4 low to drop out tcc
    delay(10);
    digitalWrite(mv1,LOW); // set mv1 for 4th
    digitalWrite(mv2,HIGH); // mv2 low
    digitalWrite(mv3,LOW); // mv3 low
        delay(10);
    analogWrite(eds2,0);
    analogWrite(eds5,0);
    analogWrite(eds3,0); //set eds 3 
     analogWrite(eds1,25); //set eds 1 low to bring up line press
    lcd.setCursor(0, 3);
    lcd.print("Gear:");
    lcd.print("4th");
    Serial.println("Fourth");
}

void DFive(){
    analogWrite(eds1,200); //set eds 1 high to drop line pressure
    analogWrite(eds4,0); //set eds 4 low to drop out tcc
    delay(10);
    digitalWrite(mv1,LOW); // set mv1 for 5th
    digitalWrite(mv2,LOW); // mv2 low
    digitalWrite(mv3,LOW); // mv3 low

    analogWrite(eds1,25); //set eds 1 low to increase line pressure
    
    
    
    for (int i=0; i <= 250; i++){ //ramp eds 2 from low to high press to engage 5th
    analogWrite(eds2, i);
    delay(1);
   }
    delay(100);
   // analogWrite(eds2,200);

    analogWrite(eds5,0);
    analogWrite(eds3,0); //set eds 3 
    lcd.setCursor(0, 3);
    lcd.print("Gear:");
    lcd.print("5th");
    Serial.println("Fifth");
}




int UPShift() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(upshift);

  // check to see if you just pressed the button 
  // (i.e. the input went from LOW to HIGH),  and you've waited 
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  } 
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    buttonState = reading;
  }
  
  // set the LED using the state of the button:
//  digitalWrite(ledPin, buttonState);

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
  
  return reading;
  
}

int DNShift() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(dnshift);

  // check to see if you just pressed the button 
  // (i.e. the input went from LOW to HIGH),  and you've waited 
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  } 
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    buttonState = reading;
  }
  
  // set the LED using the state of the button:
//  digitalWrite(ledPin, buttonState);

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
  
  return reading;
  
}

