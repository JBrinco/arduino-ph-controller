 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////pH controller with calibration//////////////////////////////////////////////////////////////////

// Email me: j.brinco@campus.fct.unl.pt
// I recommend reading the whole code with comments to get an understanding of what is going on. It is very straightforward
// I have tried to put as many values as possible in global variables, so that you can change them. For example: instead of deciding on a fixed delay between measurements, I have made some variables you can change according to your needs and whims :)
// Currently this works with 3 calibration points. Changing to more or less is not hard, but might require changing a bit of the code. Send me an e-mail if you need help.
// This script used to have two calibration ranges: one for acid and another for basic measurements. However, with our equipment, the calibration curve is so good that it became unnecessary. If you want to implement that and don't know how, just send me an email and I'll help you!

////////////////CHANGE THESE VARIABLES ACCORDING TO YOUR NEED///////////////////////////////////////////////

const int analogInPin[] ={A1, A1};     //Analog pH signal for sensor A and B. Make sure you plug it correctly, or change to another number if you really must
float ph_trigger_a = 8.0;   //The value of pH in sensor A after which something is done. You have to change the variable below to set if the system acts when the solution goes above or below this value.
bool a_control_up = true; //If set to true, the system will turn the valve on when the pH goes above ph_trigger_a, to add acid. If set to false, it will turn the valve on when the pH goes below ph_trigger_a, to add base.
float ph_trigger_b = 5.0;    //Same as above, for sensor B.
bool b_control_up = false; //Exactly the same as above. If you want to stop the solution from going above ph_trigger_b, set to true.
float cal_solution_ph[] = {4.01, 7, 9.21}; //Your calibration solution pH's, by the order they are asked for. Mathematically it does not matter what you use or the order, but I do not recommend using only acid or basic values, nor drastic chenges (going from pH 4 direcly to 9). In order to calibrate with more points, some change to the code is necessary. Shoot me an e-mail if you have difficulties.
bool need_calibration = true; //Will calibrate when the instrument is turned on. The system will not work with ought calibration!
int measure_delay = 100;    //The amount of time between measurements (in milisecs). No need to change this, unless you run into problems.
int seconds_waiting = 20;    // The amount of time it waits for you to put the meter in the calibration solution. Increase if you are slow, there is no shame :) The count down will be shown in the LCD: 20...19...18......
int delay_for_ph_reading = 1000;  //The amount of time the system waits after it has read the pH and before it opens any valves. Increase if you are having trouble reading the pH measurments because they change too fast.
float accepted_error = 0.990; // The minimum value for the coefficient of determination before the script recommends that you re-calibrate
int valve_open_time = 1000; //The time in milisecs that the valve will be turned on after a reading outside the desired range is obtained. VERY IMPORTANT TO OPTIMIZE.
int equilibration_time = 3000; //The time after the acid or base solution has been added before doing another measurement. This prevents putting too much acid or base, due to a pre-equilibrium measurement.



//General variable assignment. Don't mess with these unless you know what you are doing!

int solution_a_valve = 7;
int solution_b_valve = 8;
unsigned long int sumvalue;
int temp,cal_temp;
int analog_values[10];
float cal_voltage[3];
int cal_analog_values[10];
float sum_ph; float sum_v; float sum_ph2; float sum_v2; float sum_ph_v; float sstot; float ssres; float slope[2]; float origin[2];
int points =3;
float est_ph[3];
float rsquared[2];
float average_ph;
float phvalue[2];





#include <LiquidCrystal.h>


LiquidCrystal  lcd(12, 11, 5, 4, 3, 2);


void setup() {
 Serial.begin(9600);




pinMode(solution_a_valve, OUTPUT);
pinMode(solution_b_valve, OUTPUT);

digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, LOW);


 lcd.begin(16,2);

lcd.home ();





}
void loop() {





if (need_calibration)
  {

for(int p=0;p<2;p++){


  lcd.setCursor(0,0);
  lcd.print("I need to cali-");
  lcd.setCursor(0,1);
  lcd.print("brate meter ");
  lcd.print(p+1);
  delay(3000);
  lcd.clear();



  for(int a=seconds_waiting;a>0;a--)
    {
    lcd.setCursor(0,0);
    lcd.print("place pH ");
    lcd.print(cal_solution_ph[0]);
    lcd.setCursor(0,1);
    lcd.print("Read in ");
    lcd.print(a);
    lcd.print(" sec");
    delay(1000);
    lcd.clear();
      lcd.print("Reading");
    }

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
  cal_voltage[0]=(float)sumvalue*5.0/1024/6;
  delay(1000);



  for(int i=seconds_waiting;i>0;i--)
    {
    lcd.setCursor(0,0);
    lcd.print("place pH ");
    lcd.print(cal_solution_ph[1]);
    lcd.setCursor(0,1);
    lcd.print("Read in ");
    lcd.print(i);
    lcd.print(" sec");
    delay(1000);
    lcd.clear();
      lcd.print("Reading");
    }

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
    cal_voltage[1]=(float)sumvalue*5.0/1024/6;

  delay(1000);




  for(int i=seconds_waiting;i>0;i--)
    {
    lcd.setCursor(0,0);
    lcd.print("place pH ");
      lcd.print(cal_solution_ph[2]);
    lcd.setCursor(0,1);
    lcd.print("Read in ");
    lcd.print(i);
    lcd.print(" sec");
    delay(1000);
    lcd.clear();
    lcd.print("Reading");
    }

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

  delay(1000);










  for(int c=0, d=0;  (c<3) && (d<3); c++, d++)
    {
      sum_ph+=cal_solution_ph[c];
      sum_v+=cal_voltage[d];
      sum_ph2+=cal_solution_ph[c] * cal_solution_ph[c];
      sum_v2+=cal_voltage[d] * cal_voltage[d];
      sum_ph_v = sum_ph_v + (cal_solution_ph[c] * cal_voltage[d]);

    }



  slope[p] = ((points * sum_ph_v) - (sum_ph*sum_v)) / ((points * sum_v2) - (sum_v * sum_v));
  origin[p] = ((sum_v2 * sum_ph) - (sum_v * sum_ph_v)) / ((points * sum_v2) - (sum_v * sum_v));



  for(int y=0;  (y<3); y++)
    {
      est_ph[y] = (slope[p] * cal_voltage[y]) + origin[p];
      average_ph = sum_ph / points;

    }

  for(int r=0;  (r<3); r++)
    {
    sstot= sstot + ((cal_solution_ph[r] - average_ph)*(cal_solution_ph[r] - average_ph));
    ssres = ssres + ((cal_solution_ph[r] - est_ph[r])*(cal_solution_ph[r] - est_ph[r]));

    }

  rsquared[p] = 1- (ssres / sstot);





  if (rsquared[p]<accepted_error)
    {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Recalibration");
    lcd.setCursor(0,1);
    lcd.print("Recommended");
    lcd.print(cal_voltage[0]);
    delay(5000);
    lcd.clear();
    }


































  lcd.setCursor(0,0);
  lcd.print("slope = ");
  lcd.print(slope[p], 3);
  lcd.setCursor(0,1);
  lcd.print("origin = ");
  lcd.print(origin[p], 3);
  delay(5000);
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Coef det (R2) = ");
  lcd.setCursor(0,1);
  lcd.print(rsquared[p], 9);
  delay(5000);
  lcd.clear();



      sum_ph=0;
      sum_v=0;
      sum_ph2=0;
      sum_v2 = 0;
      sum_ph_v = 0;
}
  need_calibration = false;
}









for(int q=0;q<2;q++)
  {

 for(int i=0;i<10;i++)
  {
  analog_values[i]=analogRead(analogInPin[q]);
  delay (measure_delay);







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
 float voltage=(float)sumvalue*5.0/1024/6;
 phvalue[q] = (slope[q] * voltage) + origin[q];


  }





   Serial.print("pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\t pH B = ");
  Serial.print(phvalue[1], 3);


  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.setCursor(0,1);
  lcd.print("pH B= ");
  lcd.print(phvalue[1]);


delay(delay_for_ph_reading);












 if (phvalue[0] > ph_trigger_a and phvalue[1] > ph_trigger_b) {

if (a_control_up and b_control_up){

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, HIGH);



  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
  Serial.print("\tCorrecting pH! ");



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
digitalWrite(solution_b_valve, LOW);

 delay(equilibration_time/2);


  Serial.print("Waiting for equilibrium in A and B ");


  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Equilibrating");
  lcd.setCursor(0,1);
  lcd.print("Equilibrating");

  delay(equilibration_time/2);


 }


if (! a_control_up and b_control_up){


digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, HIGH);


  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
    Serial.print("\tCorrecting pH! ");



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
digitalWrite(solution_b_valve, LOW);

 delay(equilibration_time/2);


  Serial.print("Waiting for equilibrium in B ");


  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.setCursor(0,1);
  lcd.print("Equilibrating");


  delay(equilibration_time/2);



}


if (a_control_up and ! b_control_up){

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, LOW);


  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);



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
digitalWrite(solution_b_valve, LOW);

 delay(equilibration_time/2);


  Serial.print("Waiting for equilibrium in A");


  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Equilibrating");
  lcd.setCursor(0,1);
   lcd.print("pH B= ");
  lcd.print(phvalue[1]);

  delay(equilibration_time/2);


}



 }



 if (phvalue[0] < ph_trigger_a and phvalue[1] < ph_trigger_b) {

if (! a_control_up and ! b_control_up){

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, HIGH);



  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
  Serial.print("\tCorrecting pH! ");



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
digitalWrite(solution_b_valve, LOW);

 delay(equilibration_time/2);


  Serial.print("Waiting for equilibrium in A and B ");


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


  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
    Serial.print("\tCorrecting pH! ");



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
digitalWrite(solution_b_valve, LOW);

 delay(equilibration_time/2);


  Serial.print("Waiting for equilibrium in B ");


  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.setCursor(0,1);
  lcd.print("Equilibrating");


  delay(equilibration_time/2);



}


if (! a_control_up and b_control_up){

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, LOW);


  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);



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
digitalWrite(solution_b_valve, LOW);

 delay(equilibration_time/2);


  Serial.print("Waiting for equilibrium in A");


  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Equilibrating");
  lcd.setCursor(0,1);
   lcd.print("pH B= ");
  lcd.print(phvalue[1]);

  delay(equilibration_time/2);


}



 }



if (phvalue[0] < ph_trigger_a and phvalue[1] > ph_trigger_b) {

if (! a_control_up and b_control_up){

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, HIGH);



  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
  Serial.print("\tCorrecting pH! ");



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
digitalWrite(solution_b_valve, LOW);

 delay(equilibration_time/2);


  Serial.print("Waiting for equilibrium in A and B ");


  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Equilibrating");
  lcd.setCursor(0,1);
  lcd.print("Equilibrating");

  delay(equilibration_time/2);


 }


if (! a_control_up and b_control_up){


digitalWrite(solution_a_valve, LOW);
digitalWrite(solution_b_valve, HIGH);


  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
    Serial.print("\tCorrecting pH! ");



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
digitalWrite(solution_b_valve, LOW);

 delay(equilibration_time/2);


  Serial.print("Waiting for equilibrium in B ");


  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.setCursor(0,1);
  lcd.print("Equilibrating");


  delay(equilibration_time/2);



}


if (! a_control_up and ! b_control_up){

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, LOW);


  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);



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
digitalWrite(solution_b_valve, LOW);

 delay(equilibration_time/2);


  Serial.print("Waiting for equilibrium in A");


  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Equilibrating");
  lcd.setCursor(0,1);
   lcd.print("pH B= ");
  lcd.print(phvalue[1]);

  delay(equilibration_time/2);


}



}






if (phvalue[0] > ph_trigger_a and phvalue[1] < ph_trigger_b) {

if ( a_control_up and ! b_control_up){

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, HIGH);



  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
  Serial.print("\tCorrecting pH! ");



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
digitalWrite(solution_b_valve, LOW);

 delay(equilibration_time/2);


  Serial.print("Waiting for equilibrium in A and B ");


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


  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);
    Serial.print("\tCorrecting pH! ");



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
digitalWrite(solution_b_valve, LOW);

 delay(equilibration_time/2);


  Serial.print("Waiting for equilibrium in B ");


  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("pH A= ");
  lcd.print(phvalue[0]);
  lcd.setCursor(0,1);
  lcd.print("Equilibrating");


  delay(equilibration_time/2);



}



if (a_control_up and b_control_up){

digitalWrite(solution_a_valve, HIGH);
digitalWrite(solution_b_valve, LOW);


  Serial.print("\n pH A = ");
  Serial.print(phvalue[0], 3);
  Serial.print("\tCorrecting pH! ");
   Serial.print("\n pH B = ");
  Serial.print(phvalue[1], 3);



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
digitalWrite(solution_b_valve, LOW);

 delay(equilibration_time/2);


  Serial.print("Waiting for equilibrium in A");


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
