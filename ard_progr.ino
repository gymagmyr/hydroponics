#include <Servo.h>            //χρειαζεται για το τάισμα ψαριών
#include <DHT.h>              //χρειαζεται για το θερμόμετρο αέρα
#include <RTClib.h>           //χρειαζεται για το ρολόι

RTC_DS3231  rtc;
DHT dht22(11, DHT22);
Servo s;

const int pinPumpRelay = 3;
const int pinLampRelay = 4;
const int pinPowerLed = 9;
const int pinServo = 10;

const int pinLight = A0;
const int pinWaterLevel = A1;

const int airTempLimit = 15;
const int lightLimit = 90;

bool curLampSt = false;
bool curPumpSt = false;
int numFood;
int curLight;
float airtemp;
int prevmin;
int prevhour;

DateTime now;
char daysOfTheWeek[7][12] = {"ΚΥΡ", "ΔΕΥ", "ΤΡΙ", "ΤΕΤ", "ΠΕΜ", "ΠΑΡ", "ΣΑΒ"};


void setup() {
  Serial.begin(9600) ;

  pinMode(pinPowerLed, OUTPUT);
  pinMode(pinPumpRelay, OUTPUT);
  pinMode(pinLampRelay, OUTPUT);

  pinMode(pinLight, INPUT);
  pinMode(pinWaterLevel, INPUT);

  numFood = 0;
  curLight = 0;
  airtemp = 0;
  prevmin = 0;
  prevhour = 23;

  if (! rtc.begin())
  {
    Serial.println("Couldn't find RTC Module");
    while (1);
  }

  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  digitalWrite(  pinLampRelay , HIGH);
  digitalWrite(  pinPumpRelay , LOW);

}


void loop() {

  now = rtc.now();

  Serial.println(" Η συσκευή λειτουργεί! :)");
  digitalWrite(  pinPowerLed , HIGH);
  showDate();
  showDay();
  showTime();
  air();
  light();
  pump();
  waterLevel();
  food();
  delay(53000);   //περίμενε 53 δευτερόλεπτα


}


void air() {
  //πάρε τις μετρήσεις
  airtemp = dht22.readTemperature();
  float h = dht22.readHumidity();

  // εμφάνισέ τες στο serial monitor
  Serial.print(" Θερμοκρασία αέρα = ");
  Serial.print(airtemp);
  Serial.print(" *C ");
  Serial.print("    Υγρασία = ");
  Serial.print(h);
  Serial.print(" % ");
}


void light() {
  curLight = analogRead(pinLight);  //πάρε τις μετρήσεις
  Serial.print("  Φωτεινότητα= ");
  Serial.print(curLight);

  //αν είναι απόγευμα ή πρωί με συννεφιά
  if ((now.hour() > 14 && now.hour() <= 19) || (now.hour() >= 7 && now.hour() <= 14 && curLight < lightLimit )) {
    //άναψε λαμπα
    Serial.print("  Η λάμπα άναψε ");
    digitalWrite( pinLampRelay  , LOW);
    curLampSt = true;
  }
  //αν είναι ηλιόλουστο πρωινό ή δεν έχει ξημερώσει
  if ((now.hour() >= 7 && now.hour() <= 14 && curLight >= lightLimit ) || (now.hour() < 7 ) ) {
    //σβησε λάμπα
    Serial.print("  Η λάμπα έσβησε ");
    digitalWrite(  pinLampRelay , HIGH);
    curLampSt = false;
  }
}


void food() {
  //αν η ώρα είναι 12:16 ξεκίνα την περιστροφή του δοχείου με την τροφή ψαριών
  if (now.hour() == 12 && now.minute() == 16 ) {
    s.attach(pinServo);
    //κάνε μια στροφή
    s.write(150);
    delay(5000);
    //σταμάτα
    s.write(90);
    s.detach();
    numFood += 1;
    Serial.print("  # Food= ");
    Serial.println(numFood);
  }
}


void pump() {
  if (curPumpSt == true) {
    Serial.print("  Η αντλία δουλεύει ");
  }
  //αν πέρασε μια ώρα
  if (abs(now.hour() - prevhour) >= 1) {
    //αποθήκευσε την ώρα και το λεπτό που ξεκίνησε η αντλία
    prevhour = now.hour();
    prevmin = now.minute();

    Serial.print("  Η αντλία ξεκίνησε ");
    digitalWrite( pinPumpRelay  , LOW);   //ξεκίνα αντλία
    curPumpSt = true;
  }

  //αν η αντλία λειτουργεί και έχουν περάσει 10 λεπτά από τότε που ξεκίνησε
  if (curPumpSt == true && abs(prevmin - now.minute()) > 10) {
    Serial.print("  Η αντλία σταμάτησε ");
    digitalWrite(  pinPumpRelay , HIGH);  //σταμάτα αντλία
    curPumpSt = false;
  }
}


void waterLevel() {

  //πάρε τις μετρήσεις
  float w = analogRead(pinWaterLevel);

  Serial.print("  Επίπεδο=");
  Serial.print(w);
  if (w <= 300) {
    Serial.println("  : σχεδόν άδειο ");
  } else if (w > 300 && w <= 330) {
    Serial.println(" : Μισογεμάτο! ");
  } else if (w > 330) {
    Serial.println(" : ΓΕΜΑΤΟ! ");
  }
}



void showDate()
{
  Serial.print("    ");
  Serial.print(now.day());
  Serial.print('/');
  Serial.print(now.month());
  Serial.print('/');
  Serial.print(now.year());
  Serial.print("    ");
}
void showDay()
{

  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
}
void showTime()
{

  Serial.print("  Ώρα: ");
  Serial.print(now.hour());
  Serial.print(':');
  Serial.print(now.minute());
  Serial.print(':');
  Serial.print(now.second());
  Serial.print("    ");
}
