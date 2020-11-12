 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////pH controller with calibration//////////////////////////////////////////////////////////////////

// Email me: j.brinco@campus.fct.unl.pt

////////////////CHANGE THESE VARIABLES ACCORDING TO YOUR NEED///////////////////////////////////////////////

const int analogInPin[] ={A2, A2};     //Analog signal for sensor A and B. Make sure you plug it correctly, or change to another number if you really must
int measure_delay = 50;    //The amount of time between measurements (in milisecs). No need to change this, unless you run into problems.
int delay_for_volt_reading = 1400;  //Ammount of time between



//General variable assignment. Don't mess with these unless you know what you are doing!

int analog_values[10];     //An array of 10 analog values taken
float sumvalue;
int temp;
float voltage;

///////LCD STUFF///////

#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins. This is standard.
LiquidCrystal  lcd(12, 11, 5, 4, 3, 2);


void setup() {
 Serial.begin(9600); //Begins the serial for output. The system is designed to use the LCD, but you can still get your readings on the serial!




 lcd.begin(16,2); // Initializing LCD. The size is 16 by 2.
 lcd.home (); // Setting Cursor at Home i.e. 0,0




}
void loop() {

/////////////////INITIAL MESSAGE/////////////////////////

  Serial.print("Hello There! This is calibration of chip A ");
  Serial.print("\n The voltage of pH=7 should be around 2.5");
  Serial.print("\n Slowly adjust the potenciometer nearest to the pH probe connection");
  Serial.print("\n Once the value is acceptable, the system will tell you to stop");
  Serial.print("\n Please place probe A into solution pH=7");

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calib of chip A");
  lcd.setCursor(0,1);
  lcd.print("Place in pH=7");
  delay(14000);


 ///////////CALIBRATION OF FIRST METER////////////////////////////////////////////


while(voltage < 2.49 || voltage > 2.51){

 for(int i=0;i<10;i++)
  {
  analog_values[i]=analogRead(analogInPin[0]);
  delay(measure_delay);

  }

 for(int i=0;i<9;i++)
 {
  for(int j=i+1;j<10;j++)
  {
   if(analog_values[i]>analog_values[j])
   {
    temp=analog_values[i];
    analog_values[i]=analog_values[j];
    analog_values[j]=temp;
   }
  }
 }

 sumvalue=0;

 for(int i=2;i<8;i++)
 sumvalue+=analog_values[i];
 voltage=sumvalue*5.0/1024/6;



//Print to serial
  Serial.print("\n Voltage = ");
  Serial.print(voltage, 3);

//Print to LCD both outputs, one on the top, the other on the bottom!
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Volt = ");
  lcd.print(voltage, 3);


delay(delay_for_volt_reading);  //this ammount of time is used for you to be able to read the output


}

  Serial.print("\n GOOD! The value is set!");
  lcd.setCursor(0,1);
  lcd.print("Calib Done!");
  delay(5000);








///////////CALIBRATION OF SECOND METER////////////////////////////////////////////

  Serial.print("Calibration of chip B ");
  Serial.print("\n The voltage of pH=7 should be around 2.5");
  Serial.print("\n Slowly adjust the potenciometer nearest to the pH probe connection");
  Serial.print("\n Once the value is acceptable, the system will tell you to stop");
  Serial.print("\n Please place probe B into solution pH=7");

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calib of chip B");
  lcd.setCursor(0,1);
  lcd.print("Place in pH=7");
  delay(14000);





while(voltage < 2.49 || voltage > 2.51){

 for(int i=0;i<10;i++)
  {
  analog_values[i]=analogRead(analogInPin[1]);
  delay(measure_delay);

  }

 for(int i=0;i<9;i++)
 {
  for(int j=i+1;j<10;j++)
  {
   if(analog_values[i]>analog_values[j])
   {
    temp=analog_values[i];
    analog_values[i]=analog_values[j];
    analog_values[j]=temp;
   }
  }
 }

 sumvalue=0;

 for(int i=2;i<8;i++)
 sumvalue+=analog_values[i];
 voltage=sumvalue*5.0/1024/6;



//Print to serial
  Serial.print("\n Voltage = ");
  Serial.print(voltage, 3);

//Print to LCD both outputs, one on the top, the other on the bottom!
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Volt = ");
  lcd.print(voltage, 3);


delay(delay_for_volt_reading);  //this ammount of time is used for you to be able to read the output

}

  Serial.print("\n GOOD! The value is set!");
  lcd.setCursor(0,1);
  lcd.print("Calib Done!");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("You can now");
   lcd.setCursor(0,1);
  lcd.print("use the system!");
  delay(1000000);







}
