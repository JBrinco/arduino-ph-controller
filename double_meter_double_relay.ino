 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////pH controller with calibration//////////////////////////////////////////////////////////////////

// Email me: j.brinco@campus.fct.unl.pt
// I recommend reading the whole code with comments to get an understanding of what is going on. It is very straightforward
// I have tried to put as many values as possible in global variables, so that you can change them. For example: instead of deciding on a fixed delay between measurements, I have made some variables you can change according to your needs and whims :)
// Currently this works with 3 calibration points. Changing to more or less is not hard, but might require changing a bit of the code. Send me an e-mail if you need help.
// This script used to have two calibration ranges: one for acid and another for basic measurements. However, with our equipment, the calibration curve is so good that it became unnecessary. If you want to implement that and don't know how, just send me an email and I'll help you!

////////////////CHANGE THESE VARIABLES ACCORDING TO YOUR NEED///////////////////////////////////////////////

const int analogInPin[] ={A1, A2};     //Analog pH signal for sensor A and B. Make sure you plug it correctly, or change to another number if you really must
float ph_trigger_a = 5.0;   //The value of pH in sensor A after which something is done. You have to change the variable below to set if the system acts when the solution goes above or below this value.
bool a_control_up = true; //If set to true, the system will turn the valve on when the pH goes above ph_trigger_a, to add acid. If set to false, it will turn the valve on when the pH goes below ph_trigger_a, to add base.
float ph_trigger_b = 5.0;    //Same as above, for sensor B.
bool b_control_up = true; //Exactly the same as above. If you want to stop the solution from going above ph_trigger_b, set to true.
float cal_solution_ph[] = {4.01, 7, 9.21}; //Your calibration solution pH's, by the order they are asked for. Mathematically it does not matter what you use or the order, but I do not recommend using only acid or basic values, nor drastic chenges (going from pH 4 direcly to 9). In order to calibrate with more points, some change to the code is necessary. Shoot me an e-mail if you have difficulties.
int measure_delay = 100;    //The amount of time between measurements (in milisecs). No need to change this, unless you run into problems.
int seconds_waiting = 40;    // The amount of time it waits for you to put the meter in the calibration solution. It is VERY importat that the probe has time to stabilize before the measurement is taken. Increase if you are slow, there is no shame :) The count down will be shown in the LCD: 20...19...18......
int delay_for_ph_reading = 1000;  //The amount of time the system waits after it has read the pH and before it opens any valves. Increase if you are having trouble reading the pH measurments because they change too fast.
float accepted_error = 0.990; // The minimum value for the coefficient of determination before the script recommends that you re-calibrate
int valve_open_time = 1000; //The time in milisecs that the valve will be turned on after a reading outside the desired range is obtained. VERY IMPORTANT TO OPTIMIZE.
int equilibration_time = 3000; //The time after the acid or base solution has been added before doing another measurement. This prevents putting too much acid or base, due to a pre-equilibrium measurement.



//General variable assignment. Don't mess with these unless you know what you are doing!

bool need_calibration = true; //Will calibrate when the instrument is turned on. The system will not work with ought calibration!
int solution_a_valve = 7;    //The relay that responds to changes in pH sensor A is connected to digital pin 7 of the arduino. You can change this.
int solution_b_valve = 8;   //The relay that responds to changes in pH sensor B is connected to digital pin 8 of the arduino. You can change this as well.
unsigned long int sumvalue;     //Sum value of 10 analog measures taken
int temp,cal_temp;   //These variables are used for sorting the array when smoothing the detected analog input, and removing outliers
int analog_values[10];     //An array of 10 analog values taken
float cal_voltage[3];   //The measured voltages for the calibration solutions!
int cal_analog_values[10];   //Same as analog_values, but this variable is only used for calibration
float sum_ph; float sum_v; float sum_ph2; float sum_v2; float sum_ph_v; float sstot; float ssres; float slope[2]; float origin[2];  //Variables used in the calibration
int points =3;  //number of points in calibration. Don't just change this number and put more values in cal_solution_ph and expect it to work...
float est_ph[3]; //for r squared calculation
float rsquared[2];
float average_ph;
float phvalue[2];



///////LCD STUFF///////

#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins. This is standard.
LiquidCrystal  lcd(12, 11, 5, 4, 3, 2);


void setup() {
 Serial.begin(9600); //Begins the serial for output. The system is designed to use the LCD, but you can still get your readings on the serial!


////OUTPUT FOR RELAY////////

pinMode(solution_a_valve, OUTPUT); // Setting the output for the pin that controls the relay for sensor A
pinMode(solution_b_valve, OUTPUT);  //Setting the output for the pin that controls the relay for sensor B

digitalWrite(solution_a_valve, LOW);   //The relay "off" mode is receiving current. The "on" mode is not receiving current. You have to connect the valve accordingly in the other side of the relay (normally open or normally closed).
digitalWrite(solution_b_valve, LOW);  //Same






}
void loop() {


lcd.begin(16,2); // Initializing LCD. The size is 16 by 2.
lcd.home (); // Setting Cursor at Home i.e. 0,0



////////////////////////////////////////////////////////////////////////
////////////////////////////CALIBRATION/////////////////////////////


if (need_calibration)     //Entire calibration is nested in this if loop. After it calibrates, it sets need_calibration to false.
  {

for(int p=0;p<2;p++){


  lcd.setCursor(0,0);                //Initial message requesting calibration (Change to a more polite message when sharing the code. Or don't. I like it this way.)
  lcd.print("I need to cali-");
  lcd.setCursor(0,1);
  lcd.print("brate meter ");
  lcd.print(p+1);
  Serial.print("I need to calibrate meter ");
  Serial.print(p+1);
  delay(3000);
  lcd.clear();



  for(int i=seconds_waiting;i>0;i--)                //first pH read
    {
    Serial.print("\n Place pH  ");
    Serial.print(cal_solution_ph[0]);
    Serial.print("\t Reading in ");
    Serial.print(i);
    Serial.print("Seconds");
    lcd.setCursor(0,0);
    lcd.print("place pH ");
    lcd.print(cal_solution_ph[0]);
    lcd.setCursor(0,1);
    lcd.print("Read in ");
    lcd.print(i);
    lcd.print(" sec");
    delay(1000);
    lcd.clear();
    }

   Serial.print("\n Reading");
   lcd.print("Reading");


  for(int b=0;b<10;b++)  //Makes 10 analog reads, waiting however many milisecs you set in "measure_delay" between each reading, and stores them in array cal_analog_values
    {
    cal_analog_values[b]=analogRead(analogInPin[p]);
    delay(measure_delay);

    //lcd.setCursor(0,0);                   //This block is here for debuging purposes. Use it to check the analog value reading
    //lcd.print("Analog Value");
    //lcd.setCursor(0,1);
    //lcd.print(cal_analog_values[b]);
    //delay(1000);
    }

  for(int i=0;i<9;i++)   //this double for loop only organizes the values in the array analog_values[i] from smallest to largest!
    {
      for(int j=i+1;j<10;j++)
      {
      if(cal_analog_values[i]>cal_analog_values[j])
        {
        cal_temp=cal_analog_values[i];
        cal_analog_values[i]=cal_analog_values[j];
        cal_analog_values[j]=cal_temp;
        }
      }
    }

  sumvalue=0;
  for(int i=2;i<8;i++)   //Because the array is sorted, it removes the 2 smallest and 2 largest values (index [0,1,8,9]). The sorting above and this only serve to smooth de output value, and remove outliers!
  sumvalue+=cal_analog_values[i]; //Adds each value of the array
  cal_voltage[0]=(float)sumvalue*5.0/1024/6;  //Converts analog to voltage. It is divided by 6 to obtain the average value, because we are converting the sum of 6 analog values
  delay(1000);   //voltage reading for the first calibration solution is stored in cal_voltage[0]


  lcd.clear();

  for(int i=seconds_waiting;i>0;i--)    //Reading for the second calibration solution!
    {
    Serial.print("\n Place pH  ");
    Serial.print(cal_solution_ph[1]);
    Serial.print("\t Reading in ");
    Serial.print(i);
    Serial.print("Seconds");
    lcd.setCursor(0,0);
    lcd.print("place pH ");
    lcd.print(cal_solution_ph[1]);
    lcd.setCursor(0,1);
    lcd.print("Read in ");
    lcd.print(i);
    lcd.print(" sec");
    delay(1000);
    lcd.clear();
    }
  Serial.print("\n Reading");
  lcd.print("Reading");

  for(int b=0;b<10;b++)                            ///Everything is equal to above, except that the final value is stored in the second index of the array cal_voltage
    {
    cal_analog_values[b]=analogRead(analogInPin[p]);
    delay(measure_delay);
    }

  for(int i=0;i<9;i++)
    {
    for(int j=i+1;j<10;j++)
      {
        if(cal_analog_values[i]>cal_analog_values[j])
          {
            cal_temp=cal_analog_values[i];
            cal_analog_values[i]=cal_analog_values[j];
            cal_analog_values[j]=cal_temp;
          }
        }
    }
   sumvalue=0;
  for(int i=2;i<8;i++)
    sumvalue+=cal_analog_values[i];
    cal_voltage[1]=(float)sumvalue*5.0/1024/6;

  delay(1000);   //voltage reading for second calibration solution is stored in cal_voltage[1]




  for(int i=seconds_waiting;i>0;i--)     //Exactly the same, for the third calibration solution
    {
    Serial.print("\n Place pH  ");
    Serial.print(cal_solution_ph[2]);
    Serial.print("\t Reading in ");
    Serial.print(i);
    Serial.print("Seconds");
    lcd.setCursor(0,0);
    lcd.print("place pH ");
      lcd.print(cal_solution_ph[2]);
    lcd.setCursor(0,1);
    lcd.print("Read in ");
    lcd.print(i);
    lcd.print(" sec");
    delay(1000);
    lcd.clear();
    }

   Serial.print("\n Reading");
   lcd.print("Reading");

  for(int b=0;b<10;b++)
    {
      cal_analog_values[b]=analogRead(analogInPin[p]);
      delay(measure_delay);
    }

  for(int i=0;i<9;i++)
    {
      for(int j=i+1;j<10;j++)
        {
        if(cal_analog_values[i]>cal_analog_values[j])
          {
            cal_temp=cal_analog_values[i];
            cal_analog_values[i]=cal_analog_values[j];
            cal_analog_values[j]=cal_temp;
           }
        }
    }
  sumvalue=0;
  for(int i=2;i<8;i++)
    sumvalue+=cal_analog_values[i];
    cal_voltage[2]=(float)sumvalue*5.0/1024/6;

  delay(1000);   //voltage reading for ph=2 is stored in cal_voltage[2]





////////////////////////////CALCULATION FOR CALIBRATION//////////////////////////////////////




  for(int c=0, d=0;  (c<3) && (d<3); c++, d++)   //volt=v This for loop reads from two different arrays: cal_solution_ph[] and cal_voltage[], to calculate some values necessary for the calibration
    {
      sum_ph+=cal_solution_ph[c];
      sum_v+=cal_voltage[d];
      sum_ph2+=cal_solution_ph[c] * cal_solution_ph[c];
      sum_v2+=cal_voltage[d] * cal_voltage[d];
      sum_ph_v = sum_ph_v + (cal_solution_ph[c] * cal_voltage[d]);

    }

  //pH = y and volt = x

  slope[p] = ((points * sum_ph_v) - (sum_ph*sum_v)) / ((points * sum_v2) - (sum_v * sum_v));
  origin[p] = ((sum_v2 * sum_ph) - (sum_v * sum_ph_v)) / ((points * sum_v2) - (sum_v * sum_v));

  //R squared calculation

  for(int y=0;  (y<3); y++)    //Variables necessary for SS calculations
    {
      est_ph[y] = (slope[p] * cal_voltage[y]) + origin[p];
      average_ph = sum_ph / points;  //This is the average of real ph values from the standard solutions

    }

  for(int r=0;  (r<3); r++) // total sum of squares (sstot) and residual sum of squares (ssres) calculation
    {
    sstot= sstot + ((cal_solution_ph[r] - average_ph)*(cal_solution_ph[r] - average_ph));
    ssres = ssres + ((cal_solution_ph[r] - est_ph[r])*(cal_solution_ph[r] - est_ph[r]));

    }

  rsquared[p] = 1- (ssres / sstot);

///////////////////////////////////////////CALIBRATION RESULTS DISPLAY



  if (rsquared[p]<accepted_error)      //Gives a warning if the coefficient of determination is not satisfactory. You set what "satisfactory" means by changing "accepted_error" at the top of the script.
    {
    Serial.print("\n Recalibration Recommended");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Recalibration");
    lcd.setCursor(0,1);
    lcd.print("Recommended");
    delay(5000);
    lcd.clear();
    }

   //For displaying the results. It will give the voltage readings for each pH and the calibration results. If you don't care about that stuff, just comment everything except the part you want


  //Serial.print("\n Reading for solution with pH = ");
  //Serial.print(cal_solution_ph[0]);
  //Serial.print("Is = ");
  //Serial.print(cal_voltage[0]);
  //Serial.print("V");

 // lcd.setCursor(0,0);
 // lcd.print("pH  ");
  //lcd.print(cal_solution_ph[0]);
  //lcd.print(" read");
 // lcd.setCursor(0,1);
 // lcd.print("Volt = ");
  //lcd.print(cal_voltage[0]);
  //delay(5000);
  //lcd.clear();


 //Serial.print("\n Reading for solution with pH = ");
  //Serial.print(cal_solution_ph[1]);
  //Serial.print("Is = ");
  //Serial.print(cal_voltage[1]);
  //Serial.print("V");

  //lcd.setCursor(0,0);
  //lcd.print("pH  ");
  //lcd.print(cal_solution_ph[1]);
  //lcd.print(" read");
  //lcd.setCursor(0,1);
  //lcd.print("Volt = ");
  //lcd.print(cal_voltage[1]);
  //delay(5000);
  //lcd.clear();


   //Serial.print("\n Reading for solution with pH = ");
  //Serial.print(cal_solution_ph[2]);
  //Serial.print("Is = ");
  //Serial.print(cal_voltage[2]);
  //Serial.print("V");

  //lcd.setCursor(0,0);
  //lcd.print("pH  ");
  //lcd.print(cal_solution_ph[2]);
  //lcd.print(" read");
  //lcd.setCursor(0,1);
  //lcd.print("Volt = ");
  //lcd.print(cal_voltage[2]);
  //delay(5000);
  //lcd.clear();


  Serial.print("\n Slope =  ");
  Serial.print(slope[p], 3);
  Serial.print("\t Origin =   ");
  Serial.print(origin[p], 3);

  lcd.setCursor(0,0);
  lcd.print("slope = ");
  lcd.print(slope[p], 3);
  lcd.setCursor(0,1);
  lcd.print("origin = ");
  lcd.print(origin[p], 3);
  delay(5000);
  lcd.clear();

  //Do not comment this block if you want to see the coefficient of determination. This is very important!!! The script will run even with an awful R2, although you do get a warning (above)

  Serial.print("\n Coeficient of determination =   ");
  Serial.print(rsquared[p], 9);

  lcd.setCursor(0,0);
  lcd.print("Coef det (R2) = ");
  lcd.setCursor(0,1);
  lcd.print(rsquared[p], 9);
  delay(5000);
  lcd.clear();



      sum_ph=0;               //Resets the variables for the second calibration. Otherwise it would just add more values when doing the second calibration.
      sum_v=0;
      sum_ph2=0;
      sum_v2 = 0;
      sum_ph_v = 0;
}
  need_calibration = false;
}

///////////////////////////////////END OF CALIBRATION////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////
/////////////////////MEASURE AND VALVE CONTROL LOOP/////////////

for(int q=0;q<2;q++)    //Makes both measurments and stores them in ph_value[0] for A and ph_value[1] for B.
  {

 for(int i=0;i<10;i++)  //Makes 10 analog reads, waiting 50 milisecs between each, and stores them in array analog_values
  {
  analog_values[i]=analogRead(analogInPin[q]);
  delay (measure_delay);

   //Serial.print("Analog Value = ");
   //Serial.print(analog_values[i]);
   // lcd.setCursor(0,0);          //Debugging!
 // lcd.print("Analog Value");   //Debugging!
 // lcd.setCursor(0,1);          //Debugging! Uncomment these five lines if you have problems and want to check your analog input. It will wait 1 sec between measurements
 // lcd.print(analog_values[i]); //Debugging!
  //delay(1000);

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

 for(int i=2;i<8;i++)   //Because the array is sorted, it removes the 2 smallest and 2 largest values (index [0,1,8,9]). The sorting above and this only serve to smooth de output value, and remove outliers!
 sumvalue+=analog_values[i]; //Adds each value of the array
 float voltage=(float)sumvalue*5.0/1024/6; // Transforms analog to digital. 1024 (Binary 111111111) corresponds to 5 volts. Divided by 6 to obtain the average (because it was a sum).
 phvalue[q] = (slope[q] * voltage) + origin[q];


  }

/////////////////////PRINTING THE MEASURMENTS!//////////////////////////


//Print to serial
  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\t pH B = ");
  Serial.print(phvalue[1], 3);

//Print to LCD both outputs, one on the top, the other on the bottom!
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.setCursor(0,1);
  lcd.print("pH B= ");
  lcd.print(phvalue[1]);


delay(delay_for_ph_reading);  //this ammount of time is used for you to be able to read the output





///////////////////////CONTROLING VALVES BASED ON PH MEASUREMENTS//////////////////////////////

//this whole section is just 4 if statements: a is above setpoint and b is also above; a is above setpoint and b is below; a is below setpoint and b is above; both are below setpoint.
//Inside each if statement there are 3 if statements, to change the behaviour according to the variables a_control_up and b_control_up. If none of these if statements are met, then the system does not open any valves



 if (phvalue[0] > ph_trigger_a and phvalue[1] > ph_trigger_b) {

if (a_control_up and b_control_up){  //Both a and b are set to control up, so opens both valves

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, HIGH);

//Print to serial

  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
  Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
  Serial.print("\tCorrecting pH! ");


//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.print(" Open");
   lcd.setCursor(0,1);
  lcd.print("pH B= ");
  lcd.print(phvalue[1]);
  lcd.print(" Open");

delay(valve_open_time);

  digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, LOW);  //Turn relays off again

 delay(equilibration_time/2);

//Print to serial
  Serial.print("Waiting for equilibrium in A and B ");

//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Equilibrating");
  lcd.setCursor(0,1);
  lcd.print("Equilibrating");

  delay(equilibration_time/2);


 }


if (! a_control_up and b_control_up){ //A is set to control down and b is set to control up. Opens only b, since a is above the setpoint, which is desired


digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, HIGH);

//Print to serial
  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
    Serial.print("\tCorrecting pH! ");


//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
   lcd.setCursor(0,1);
  lcd.print("pH B= ");
  lcd.print(phvalue[1]);
   lcd.print(" Open");

delay(valve_open_time);

  digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, LOW);   //Turn relay off again

 delay(equilibration_time/2);

//Print to serial
  Serial.print("Waiting for equilibrium in B ");

//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.setCursor(0,1);
  lcd.print("Equilibrating");


  delay(equilibration_time/2);



}


if (a_control_up and ! b_control_up){  //A is set to control up and b to control down. Opens only A

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, LOW);

//Print to serial
  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);


//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.print(" Open");
   lcd.setCursor(0,1);
  lcd.print("pH B= ");
  lcd.print(phvalue[1]);

delay(valve_open_time);

  digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, LOW);   //Turn relay off again

 delay(equilibration_time/2);

//Print to serial
  Serial.print("Waiting for equilibrium in A");

//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Equilibrating");
  lcd.setCursor(0,1);
   lcd.print("pH B= ");
  lcd.print(phvalue[1]);

  delay(equilibration_time/2);


}



 }
//////////////////////////////////////////////////////////////////////


 if (phvalue[0] < ph_trigger_a and phvalue[1] < ph_trigger_b) {     //Both are below setpoint

if (! a_control_up and ! b_control_up){

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, HIGH);

//Print to serial

  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
  Serial.print("\tCorrecting pH! ");


//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.print(" Open");
   lcd.setCursor(0,1);
  lcd.print("pH B= ");
  lcd.print(phvalue[1]);
  lcd.print(" Open");

delay(valve_open_time);

  digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, LOW);  //Turn relays off again

 delay(equilibration_time/2);

//Print to serial
  Serial.print("Waiting for equilibrium in A and B ");

//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Equilibrating");
  lcd.setCursor(0,1);
  lcd.print("Equilibrating");

  delay(equilibration_time/2);


 }


if (a_control_up and ! b_control_up){


digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, HIGH);

//Print to serial
  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
    Serial.print("\tCorrecting pH! ");


//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
   lcd.setCursor(0,1);
  lcd.print("pH B= ");
  lcd.print(phvalue[1]);
   lcd.print(" Open");

delay(valve_open_time);

  digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, LOW);   //Turn relay off again

 delay(equilibration_time/2);

//Print to serial
  Serial.print("Waiting for equilibrium in B ");

//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.setCursor(0,1);
  lcd.print("Equilibrating");


  delay(equilibration_time/2);



}


if (! a_control_up and b_control_up){  //A is set to control up and b to control down. Opens only A

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, LOW);

//Print to serial
  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);


//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.print(" Open");
   lcd.setCursor(0,1);
  lcd.print("pH B= ");
  lcd.print(phvalue[1]);

delay(valve_open_time);

  digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, LOW);   //Turn relay off again

 delay(equilibration_time/2);

//Print to serial
  Serial.print("Waiting for equilibrium in A");

//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Equilibrating");
  lcd.setCursor(0,1);
   lcd.print("pH B= ");
  lcd.print(phvalue[1]);

  delay(equilibration_time/2);


}



 }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

if (phvalue[0] < ph_trigger_a and phvalue[1] > ph_trigger_b) {     //A is below setpoint and b is above setpoint

if (! a_control_up and b_control_up){  //A is set to control down and b is set to control up, so opens both valves

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, HIGH);

//Print to serial

  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
  Serial.print("\tCorrecting pH! ");


//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.print(" Open");
   lcd.setCursor(0,1);
  lcd.print("pH B= ");
  lcd.print(phvalue[1]);
  lcd.print(" Open");

delay(valve_open_time);

  digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, LOW);  //Turn relays off again

 delay(equilibration_time/2);

//Print to serial
  Serial.print("Waiting for equilibrium in A and B ");

//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Equilibrating");
  lcd.setCursor(0,1);
  lcd.print("Equilibrating");

  delay(equilibration_time/2);


 }


if (! a_control_up and b_control_up){ //A is control down, b is control up . Opens only b.


digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, HIGH);

//Print to serial
  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
    Serial.print("\tCorrecting pH! ");


//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
   lcd.setCursor(0,1);
  lcd.print("pH B= ");
  lcd.print(phvalue[1]);
   lcd.print(" Open");

delay(valve_open_time);

  digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, LOW);   //Turn relay off again

 delay(equilibration_time/2);

//Print to serial
  Serial.print("Waiting for equilibrium in B ");

//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.setCursor(0,1);
  lcd.print("Equilibrating");


  delay(equilibration_time/2);



}


if (! a_control_up and ! b_control_up){  //both are set to control down. Opens only A

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, LOW);

//Print to serial
  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);


//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.print(" Open");
   lcd.setCursor(0,1);
  lcd.print("pH B= ");
  lcd.print(phvalue[1]);

delay(valve_open_time);

  digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, LOW);   //Turn relay off again

 delay(equilibration_time/2);

//Print to serial
  Serial.print("Waiting for equilibrium in A");

//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Equilibrating");
  lcd.setCursor(0,1);
   lcd.print("pH B= ");
  lcd.print(phvalue[1]);

  delay(equilibration_time/2);


}



}



/////////////////////////////////////////////////////////////////////////////////////////////////////////


if (phvalue[0] > ph_trigger_a and phvalue[1] < ph_trigger_b) {     //A is above setpoint and b is below setpoint

if ( a_control_up and ! b_control_up){  //opens both valves

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, HIGH);

//Print to serial

  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
  Serial.print("\tCorrecting pH! ");


//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.print(" Open");
   lcd.setCursor(0,1);
  lcd.print("pH B= ");
  lcd.print(phvalue[1]);
  lcd.print(" Open");

delay(valve_open_time);

  digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, LOW);  //Turn relays off again

 delay(equilibration_time/2);

//Print to serial
  Serial.print("Waiting for equilibrium in A and B ");

//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Equilibrating");
  lcd.setCursor(0,1);
  lcd.print("Equilibrating");

  delay(equilibration_time/2);


 }


if (a_control_up and ! b_control_up){ //A is control down, b is control up . Opens only b.


digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, HIGH);

//Print to serial
  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
    Serial.print("\tCorrecting pH! ");


//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
   lcd.setCursor(0,1);
  lcd.print("pH B= ");
  lcd.print(phvalue[1]);
   lcd.print(" Open");

delay(valve_open_time);

  digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, LOW);   //Turn relay off again

 delay(equilibration_time/2);

//Print to serial
  Serial.print("Waiting for equilibrium in B ");

//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.setCursor(0,1);
  lcd.print("Equilibrating");


  delay(equilibration_time/2);



}



if (a_control_up and b_control_up){  //both are set to control down. Opens only A

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, LOW);

//Print to serial
  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);


//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.print(" Open");
   lcd.setCursor(0,1);
  lcd.print("pH B= ");
  lcd.print(phvalue[1]);

delay(valve_open_time);

  digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, LOW);   //Turn relay off again

 delay(equilibration_time/2);

//Print to serial
  Serial.print("Waiting for equilibrium in A");

//Print to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Equilibrating");
  lcd.setCursor(0,1);
   lcd.print("pH B= ");
  lcd.print(phvalue[1]);

  delay(equilibration_time/2);


}



}
}
