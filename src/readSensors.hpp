#ifndef DATA_SENSORS__H
  #include "data_sensors.h"
#endif 
#include <Arduino.h>
#define PIN_TRIGGER D5 // 
#define PIN_ECHO   D6 //
#define SOUND_VELOCITY 0.034 //  CM/SEG 
#define PIN_SENSOR_CAP D2    //sensor nivel Alto 
#define PIN_SENSOR_CAP_1 D7  //sensor nivel bajo

//ultimas 5 lecturas del sensor ultrasonico  
sensor_ultrasonic sensor_distance[5];  
sensor_distance_media_values sensor_distance_media ; 
s_cap sensor_cap_min; 
s_cap sensor_cap_max ; 
void initPorts(){
    pinMode(PIN_TRIGGER,OUTPUT) ;
    pinMode(PIN_ECHO,INPUT) ; 
    pinMode(PIN_SENSOR_CAP,INPUT) ;
    pinMode(PIN_SENSOR_CAP_1,INPUT) ;

}


// lectura cada 5 segundos, lee y calcula media movil y mediana 
// con un buffer de tamaño 5 
// además, obtiene el unix_time desde un servidor NTC (falta terminar)
void readUltrasonicSensor(){ 
    int index_fifo_buffer = 0 ; 
    unsigned int distance_median[5] ; 
    //getNTC unix time ! 
    time_t unix_time = getHourNTC() ;
    if (unix_time == 0){
        unix_time = (sensor_distance[0].unix_time_sample != 0)?sensor_distance[0].unix_time_sample+5:0 ; 
    } 

    digitalWrite(PIN_TRIGGER,HIGH) ; 
    delayMicroseconds(10) ; 
    digitalWrite(PIN_TRIGGER,LOW) ;  
    //pulseIn return 0 if error response 
    unsigned long int miliseconds_response = pulseIn(PIN_ECHO,HIGH); 
    if (miliseconds_response == 0){
        // publish error sensor ! 
    }
    float distance_cm = SOUND_VELOCITY * (miliseconds_response/2) ;
    // fifo buffer rutina  
    for (index_fifo_buffer = 4;index_fifo_buffer>0;index_fifo_buffer--)
    {
        sensor_distance[index_fifo_buffer] = sensor_distance[index_fifo_buffer-1]; 
        if (sensor_distance[index_fifo_buffer].unix_time_sample == 0 ){
            sensor_distance[index_fifo_buffer].unix_time_sample = (unix_time !=(time_t) 0)?unix_time-(time_t)(5*index_fifo_buffer) :sensor_distance[index_fifo_buffer].unix_time_sample ; 
        }
        
    }
    sensor_distance[0].distance = (int) distance_cm ; 
    sensor_distance[0].unix_time_sample = unix_time ; 
    sensor_distance_media.media_movil =( sensor_distance[0].distance + 
                                         sensor_distance[1].distance +
                                         sensor_distance[2].distance + 
                                         sensor_distance[3].distance +
                                         sensor_distance[4].distance)/5.0 ; 
    // mediana  
    distance_median[0] = sensor_distance[0].distance ; 
    distance_median[1] = sensor_distance[1].distance ; 
    distance_median[2] = sensor_distance[2].distance ; 
    distance_median[3] = sensor_distance[3].distance ;    
    distance_median[4] = sensor_distance[4].distance ; 
    int aux ; 
    //ordenamiento metodo de la burbuja  
    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            if(distance_median[j] > distance_median[j + 1]){
                aux = distance_median[j];
                distance_median[j] = distance_median[j + 1];
                distance_median[j + 1] = aux;
            }
        }
    }
    sensor_distance_media.median_data = distance_median[2] ;
}


void sensorCapacitivo(){

    time_t unix_time = getHourNTC() ; 
    if (unix_time == 0){
        Serial.print(unix_time) ; 
        sensor_cap_max.last_unix_time = (sensor_cap_max.last_unix_time != 0)?sensor_cap_max.last_unix_time+(time_t )(600): sensor_cap_max.last_unix_time ;  
        sensor_cap_min.last_unix_time = (sensor_cap_min.last_unix_time != 0)?sensor_cap_min.last_unix_time+(time_t )(600): sensor_cap_min.last_unix_time ;  
    }else{ 
        sensor_cap_max.last_unix_time = unix_time ; 
        sensor_cap_min.last_unix_time = unix_time ; 
    }
     //lectura primer sensor capacitivo 
    if (digitalRead(PIN_SENSOR_CAP) == LOW) {
        sensor_cap_max.state_sensor_cap = 1 ;  // hay combustible 
    }else if (digitalRead(PIN_SENSOR_CAP) == HIGH){ 
        sensor_cap_max.state_sensor_cap = 0 ; //no hay combustible
    }
    //lectura segundo sensor capacitivo 
    if (digitalRead(PIN_SENSOR_CAP_1) == LOW) {
        sensor_cap_min.state_sensor_cap = 1 ; // hay combustible  
    }else if (digitalRead(PIN_SENSOR_CAP_1) == HIGH){ 
        sensor_cap_min.state_sensor_cap = 0 ;// no hay combustible
    }



}