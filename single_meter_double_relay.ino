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
const int analogInPin = A1;     //Analog pH signal comes from A1. Make sure you plug it correctly, or change to another number if you really must
unsigned long int sumvalue;     //Sum value of 10 analog measures taken
int temp,cal_temp;   //These variables are used for sorting the array when smoothing the detected analog input, and removing outliers
int analog_values[10];     //An array of 10 analog values taken
float cal_voltage[3];   //The measured voltages for the calibration solutions!
int cal_analog_values[10];   //Same as analog_values, but this variable is only used for calibration
float sum_ph; float sum_v; float sum_ph2; float sum_v2; float sum_ph_v; float sstot; float ssres; float slope; float origin;  //Variables used in the calibration
int points =3;  //number of points in calibration. Don't just change this number and put more values in cal_solution_ph and expect it to work...
float est_ph[3]; //for r squared calculation
float rsquared;
int too_acid = 7;    //The relay that releases basic solution (when the medium is too acid) is connected to digital pin 7 of the arduino. You can change this
int too_basic = 8;   //The relay that releases acid solution (when the medium is too basic) is connected to digital pin 8 of the arduino. You can change this as well.
float average_ph;
bool need_calibration = true; //Will calibrate when the instrument is turned on. Will not work with ought calibration!



///////LCD STUFF///////

#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins. This is standard.
LiquidCrystal  lcd(12, 11, 5, 4, 3, 2);


void setup() {
 Serial.begin(9600); //Begins the serial for output. The system is designed to use the LCD, but you can still get your readings on the serial!


////OUTPUT FOR RELAY////////

pinMode(too_acid, OUTPUT); // Setting the output for the pin that controls the base adding relay
pinMode(too_basic, OUTPUT);  //Setting the output for the pin that controls the acid adding relay

digitalWrite(too_acid, LOW);   //The relay "off" mode is receiving current. The "on" mode is not receiving current. You have to connect the valve accordingly in the other side of the relay (normally open or normally closed).
digitalWrite(too_basic, LOW);  //Same


 lcd.begin(16,2); // Initializing LCD. The size is 16 by 2.

lcd.home (); // Setting Cursor at Home i.e. 0,0





}
void loop() {


////////////////////////////////////////////////////////////////////////
////////////////////////////CALIBRATION/////////////////////////////

if (need_calibration)     //Entire calibration is nested in this if loop. After it calibrates, it sets need_calibration to false.
	{

 //Initial message requesting calibration (Change to a more polite message when sharing the code. Or don't. I like it this way.)

  Serial.print("I need to calibrate myself!");
	lcd.setCursor(0,0);               
	lcd.print("I need to cali-");
	lcd.setCursor(0,1);
	lcd.print("brate myself!");
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
		cal_analog_values[b]=analogRead(analogInPin);
		delay(measure_delay);

		//lcd.setCursor(0,0);                   //This block is here for debuging purposes. Use it to check the analog value reading
		//lcd.print("Analog Value");
		//lcd.setCursor(0,1);
		//lcd.print(cal_analog_values[b]);
		//delay(1000);
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

	slope = ((points * sum_ph_v) - (sum_ph*sum_v)) / ((points * sum_v2) - (sum_v * sum_v));
	origin = ((sum_v2 * sum_ph) - (sum_v * sum_ph_v)) / ((points * sum_v2) - (sum_v * sum_v));

	//R squared calculation

	for(int y=0;  (y<3); y++)    //Variables necessary for SS calculations
		{
  		est_ph[y] = (slope * cal_voltage[y]) + origin;
  		average_ph = sum_ph / points;  //This is the average of real ph values from the standard solutions

		}

	for(int r=0;  (r<3); r++) // total sum of squares (sstot) and residual sum of squares (ssres) calculation
		{
 		sstot= sstot + ((cal_solution_ph[r] - average_ph)*(cal_solution_ph[r] - average_ph));
 		ssres = ssres + ((cal_solution_ph[r] - est_ph[r])*(cal_solution_ph[r] - est_ph[r]));

		}

	rsquared = 1- (ssres / sstot);

///////////////////////////////////////////CALIBRATION RESULTS DISPLAY



	if (rsquared<accepted_error)      //Gives a warning if the coefficient of determination is not satisfactory. You set what "satisfactory" means by changing "accepted_error" at the top of the script.
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
  
	//lcd.setCursor(0,0);
	//lcd.print("pH  ");
	//lcd.print(cal_solution_ph[0]);
	//lcd.print(" read");
	//lcd.setCursor(0,1);
	//lcd.print("Volt = ");
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
  Serial.print(slope, 3);
  Serial.print("\t Origin =   ");
  Serial.print(origin, 3);

	lcd.setCursor(0,0);
	lcd.print("slope = ");
	lcd.print(slope, 3);
	lcd.setCursor(0,1);
	lcd.print("origin = ");
	lcd.print(origin, 3);
	delay(10000);
	lcd.clear();

//Do not comment this block if you want to see the coefficient of determination. This is very important!!! The script will run even with an awful R2, although you do get a warning (above)

  Serial.print("\n Coeficient of determination =   ");
  Serial.print(rsquared, 9);

	lcd.setCursor(0,0);                    
	lcd.print("Coef det (R2) = ");
	lcd.setCursor(0,1);
	lcd.print(rsquared, 9);
	delay(8000);
	lcd.clear();




	need_calibration = false;
}

///////////////////////////////////END OF CALIBRATION////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////
/////////////////////MEASURE AND VALVE CONTROL LOOP/////////////

 for(int i=0;i<10;i++)  //Makes 10 analog reads, waiting 50 milisecs between each, and stores them in array analog_values
 	{
	analog_values[i]=analogRead(analogInPin);
	delay (measure_delay);

 // lcd.setCursor(0,0);          //Debugging!
 // lcd.print("Analog Value");   //Debugging!
 // lcd.setCursor(0,1);          //Debugging! Uncomment these five lines if you have problems and want to check your analog input
 // lcd.print(analog_values[i]); //Debugging!
  //delay(1000);

	}

 for(int i=0;i<9;i++)   //This double for loop only organizes the values in the array analog_values[i] from smallest to largest!
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
 float phvalue = (slope * voltage) + origin;


// If pH is too high or too low, do something! Now!
 if (phvalue > ph_max) {

	digitalWrite(too_basic, HIGH);                // If the pH is too high, remove current to relay "too_basic", as defined in the variables above, whence turning it "on".


 //Print to serial
	Serial.print("pH Value = ");
	Serial.print(phvalue, 3);
	Serial.print("\tAdding acid! ");

//Print to LCD
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("pH= ");
	lcd.print(phvalue);
	lcd.setCursor(0,1);
	lcd.print("Adding Acid! ");

	delay(valve_open_time);

	digitalWrite(too_basic, LOW);   //Turn relay off again

	delay(equilibration_time/2);  //After the relay is turned off again, I divide the equilibration time in two, because otherwise there is too little time to read the actual pH value (just the time the relay is on). This way the pH value stays printed longer on the LCD.

 //Print to serial
	Serial.print("Waiting for equilibrium ");

//Print to LCD
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Waiting for ");
 	lcd.setCursor(0,1);
 	lcd.print("Equilibrium ");

	delay(equilibration_time/2);


  	}
  else if (phvalue <ph_min){

	digitalWrite(too_acid, HIGH);              //If the pH is too low, remove current to relay "too_acid", as defined in the variables above, whence turning it "on".

 //Print to serial
 	Serial.print("pH Value = ");
	Serial.print(phvalue, 3);
 	Serial.print("\tAdding Base");

//Print to LCD
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("pH= ");
	lcd.print(phvalue);
	lcd.setCursor(0,1);
	lcd.print("Adding Base! ");


	delay(valve_open_time);

	digitalWrite(too_acid, LOW);   //Turn relay off again

	delay(equilibration_time/2);

 //Print to serial
 	Serial.print("Waiting for equilibrium ");

//Print to LCD
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Waiting for ");
	lcd.setCursor(0,1);
	lcd.print("Equilibrium ");

	delay(equilibration_time/2);

  	}

 else {                                          //Otherwise set both relays to on and print the pH and voltage value. The digitalwrite is probably redundant, but it does not hurt to have it here

	digitalWrite(too_acid, LOW);
	digitalWrite(too_basic, LOW);

 //Print to serial
 	Serial.print("pH Value = ");
 	Serial.print(phvalue, 3);
  Serial.print("\t");
  Serial.print(ph_min);
  Serial.print(" < pH < ");
  Serial.print(ph_max);

//Print to LCD
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("pH= ");
	lcd.print(phvalue);
	lcd.setCursor(0,1);
	lcd.print(ph_min);
  lcd.print(" < pH < ");
	lcd.print(ph_max);

	}


delay(delay_between_measurements);// Wait a certain amount of time (in milisecs). The system should refresh the reading every x  miliseconds, plus the measure_delay * 10. (if the measure delay is 100 milisecs and this final delay is 200 milisecs, it will refresh every 1.2 seconds, roughly. This final delay can be reduced or increased to change the refresh rate.



 }
