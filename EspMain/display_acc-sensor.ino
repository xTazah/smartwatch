#if 0
TaskHandle_t Task1;
TaskHandle_t Task2;

//Abstract Device
#include <ArduinoJson.h>




DynamicJsonDocument info(1024);
AccGyroSensor sensor1("acceleration_gyro_sensor_watch"); //acc_gyro_sensor
RTC_Display sensor2("4x7 segment display with shifregister and RTC",2,15,19,4,5,18,23,12,14,13);

bool bButtonPrev; //Button for triggering the print of the acc sensor

void setup() {
  xTaskCreatePinnedToCore(
    Task1Code,   /* Task function. */
    "Task1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    Task2Code,   /* Task function. */
    "Task2",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task2,      /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */
    delay(500); 


  //Display
  Serial.begin(115200); 
  
  //Acc Sensor
  sensor1.setup();

  pinMode(25,INPUT);  //Button to trigger the printing of acc sensor data
  
  //Serial.println("");
  delay(100);

  //RTC
  sensor2.setup();

}

//Tasks
//Display + RTC
void Task1Code(void *pvParamaters){
  Serial.print("Display + RTC running on core ");
  Serial.println(xPortGetCoreID());
  for(;;){
    RtcDateTime now = sensor2.Rtc->GetDateTime();
    
    //sensor2.timer.update(); 
    delay(2);
    //sensor2.timer.stop(sensor2.timer_event); 
    sensor2.screenOff(); 

    sensor2.number = now.Hour()*100+now.Minute();
    //Serial.println(sensor2.number);
    //Serial.println(sensor2.cathodePins[0]);
    //Serial.println(sensor2.cathodePins[1]);
    //Serial.println(sensor2.cathodePins[2]);
    //Serial.println(sensor2.cathodePins[3]);
    if (sensor2.number > 9999) { 
      Serial.println("no valid time");
    } 
    else {
      sensor2.separate(sensor2.number);
      //std::function<void> func = std::bind(&RTC_Display::Display,sensor2);
      /*
      auto func = []() mutable -> void {
        sensor2.Display();
      };*/
      sensor2.Display();
      //sensor2.timer_event = sensor2.timer.every(1, func);   
    }
  }
}

void Task2Code(void *pvParameters){
  Serial.print("Acceleration sensor running on core ");
  Serial.println(xPortGetCoreID());
  
  for(;;){

    if (digitalRead(25) && !bButtonPrev){
      bButtonPrev = 1;
      sensor1.sendToServer(info);
    }
    if (!digitalRead(25)){
      bButtonPrev = 0;
    }

  }
}


void loop() {

}


#endif