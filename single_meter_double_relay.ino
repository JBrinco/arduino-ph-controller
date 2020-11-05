//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////pH controller with calibration//////////////////////////////////////////////////////////////////////

//Email me: j.brinco@campus.fct.unl.pt
//I recommend reading the whole code with comments to get an understanding of what is going on. It is very straightforward
//I have tried to put as many values as possible in global variables, so that you can change them. For example: instead of deciding on a fixed delay between measurements, I have made some variables you can change according to your needs and whims :)
//Currently this works with 3 calibration points. Changing to more or less is not hard, but might require changing a bit of the code. Send me an e-mail if you need help.



////////////////CHANGE THESE VARIABLES ACCORDING TO YOUR NEED///////////////////////////////////////////////

float ph_max = 8.0;   //Maximum pH value allowed before the relay is triggered
float ph_min = 5.0;    //Minimum pH value allowed before the other relay is triggered
float cal_solution_ph[] = {4.01, 7, 9.21}; //Your calibration solution pH's, by the order they are asked for. Mathematically it does not matter what you use or the order, but I do not recommend using only acid or basic values. In order to calibrate with more points, some change to the code is necessary. Shoot me an e-mail if you have difficulties.
int measure_delay = 100;    //The amount of time between measurements (in milisecs)
int seconds_waiting = 20;    // The amount of time it waits for you to put the meter in the calibration solution. Increase if you are slow, there is no shame :) The count down will be shown in the LCD: 20...19...18......
int delay_between_measurements = 200;  //The amount of time the system waits after it has read the pH and adjusted the relays accordingly, before starting another measurement cycle. I don't think this is terribly important, but don't make it too high, or you will compromise the measurement frequency.
float accepted_error = 0.990; // The minimum value for the coefficient of determination before the script recommends that you re-calibrate
int valve_open_time = 1000; //The time in milisecs that the relay will be turned on after a reading outside the desired range is obtained
int equilibration_time = 3000; //The time after the acid or base solution has been added before doing another measurement. This prevents putting too much acid or base, due to a pre-equilibrium measurement






//General variable assignment. Don't mess with these unless you know what you are doing!
const int analogInPin = A1;
unsigned long int sumvalue;
int temp,cal_temp;
int analog_values[10];
float cal_voltage[3];
int cal_analog_values[10];
float sum_ph; float sum_v; float sum_ph2; float sum_v2; float sum_ph_v; float sstot; float ssres; float slope; float origin;
int points =3;
float est_ph[3];
float rsquared;
int too_acid = 7;
int too_basic = 8;
float average_ph;
bool need_calibration = true;





#include <LiquidCrystal.h>


LiquidCrystal  lcd(12, 11, 5, 4, 3, 2);


void setup() {
 Serial.begin(9600);




pinMode(too_acid, OUTPUT);
pinMode(too_basic, OUTPUT);

digitalWrite(too_acid, LOW);
digitalWrite(too_basic, LOW);


 lcd.begin(16,2);

lcd.home ();





}
void loop() {





if (need_calibration)
	{


	lcd.setCursor(0,0);
	lcd.print("I need to cali-");
	lcd.setCursor(0,1);
	lcd.print("brate myself!");
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
		cal_analog_values[b]=analogRead(analogInPin);
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
		cal_analog_values[b]=analogRead(analogInPin);
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
  		cal_analog_values[b]=analogRead(analogInPin);
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



	slope = ((points * sum_ph_v) - (sum_ph*sum_v)) / ((points * sum_v2) - (sum_v * sum_v));
	origin = ((sum_v2 * sum_ph) - (sum_v * sum_ph_v)) / ((points * sum_v2) - (sum_v * sum_v));



	for(int y=0;  (y<3); y++)
		{
  		est_ph[y] = (slope * cal_voltage[y]) + origin;
  		average_ph = sum_ph / points;

		}

	for(int r=0;  (r<3); r++)
		{
 		sstot= sstot + ((cal_solution_ph[r] - average_ph)*(cal_solution_ph[r] - average_ph));
 		ssres = ssres + ((cal_solution_ph[r] - est_ph[r])*(cal_solution_ph[r] - est_ph[r]));

		}

	rsquared = 1- (ssres / sstot);





	if (rsquared<accepted_error)
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
	lcd.print(slope, 3);
	lcd.setCursor(0,1);
	lcd.print("origin = ");
	lcd.print(origin, 3);
	delay(10000);
	lcd.clear();

	lcd.setCursor(0,0);
	lcd.print("Coef det (R2) = ");
	lcd.setCursor(0,1);
	lcd.print(rsquared, 9);
	delay(8000);
	lcd.clear();




	need_calibration = false;
}



//////////////////END OF CALIBRATION////////////////////////////





 for(int i=0;i<10;i++)
 	{
	analog_values[i]=analogRead(analogInPin);
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
 float phvalue = (slope * voltage) + origin;



 if (phvalue > ph_max) {

	digitalWrite(too_basic, HIGH);



	Serial.print("pH Value = ");
	Serial.print(phvalue, 3);
	Serial.print("\tAdding acid! ");


	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("pH= ");
	lcd.print(phvalue);
	lcd.setCursor(0,1);
	lcd.print("Adding Acid! ");

	delay(valve_open_time);

	digitalWrite(too_basic, LOW);

	delay(equilibration_time/2);


	Serial.print("Waiting for equilibrium ");


	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Waiting for ");
 	lcd.setCursor(0,1);
 	lcd.print("Equilibrium ");

	delay(equilibration_time/2);


  	}
  else if (phvalue <ph_min){

	digitalWrite(too_acid, HIGH);


 	Serial.print("pH Value = ");
	Serial.print(phvalue, 3);
 	Serial.print("\tAdding Base");


	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("pH= ");
	lcd.print(phvalue);
	lcd.setCursor(0,1);
	lcd.print("Adding Base! ");


	delay(valve_open_time);

	digitalWrite(too_acid, LOW);

	delay(equilibration_time/2);


 	Serial.print("Waiting for equilibrium ");


	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Waiting for ");
	lcd.setCursor(0,1);
	lcd.print("Equilibrium ");

	delay(equilibration_time/2);

  	}

 else {

	digitalWrite(too_acid, LOW);
	digitalWrite(too_basic, LOW);


 	Serial.print("pH Value = ");
 	Serial.print(phvalue, 3);
 	Serial.print("\tVoltage= ");
	Serial.println(voltage, 3);


	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("pH= ");
	lcd.print(phvalue);
	lcd.setCursor(0,1);
	lcd.print(ph_min);
  lcd.print(" < pH < ");
	lcd.print(ph_max);

	}


delay(delay_between_measurements);



 }
