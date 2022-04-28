#include "DestinationSelection.h"

void get_angle(float* x, float* y) {
  imu.readAccelData(imu.accelCount);
  *x = imu.accelCount[0] * imu.aRes;
  *y = imu.accelCount[1] * imu.aRes;
}

void lookup(char* query, char* response) {
  const char space = ' ';
  char *building;
  char *floor;
  int space_ind;
  for(int i = 0; i< strlen(query); i ++){
    const char curChar = (query[i]);
    // if(!strcmp(curChar,space)){
    if(curChar == space){
      query[i]= '\0'; 
      space_ind = i;
      building = query;
      floor = query + space_ind +1;
      sprintf(destination, "%s", building);
      destination_floor = floor[0] - '0'; //converts that char to int    
      sprintf(response, "let's find building  %s, floor %s!", building, floor);
      break;
    }else{
      sprintf(response, "You must seperate building and floor with a space!");
    }
  }
}

DestinationSelection::DestinationSelection(){}

void DestinationSelection::update(int button){
    float x, y;
    get_angle(&x, &y); //get angle values
    char q[50];
    if(state == 0){
      strcpy(output,msg);
      if(button==2){
        char_index = 0; 
        state = 1;
        scroll_timer = millis();
      }
    }
    else if(state==1){
      if(button==1){
        char current_letter[50] = {buildings[char_index],buildings[char_index + 1]}; //gets the value at char_index
        strcat(query_string,current_letter);
        strcat(query_string, " ");
        strcpy(output,query_string);
        // char_index =0;
        state = 4;
      }else if(millis() - scroll_timer >= scroll_threshold){
        if (abs(angle) > angle_threshold){
          if(angle>0){
            if(char_index == (strlen(buildings)-2)){
              char_index = 0;
            }else{
              char_index = char_index +2; // go forward 
            }
          }
          if(angle<0){
            if(char_index==0){
              char_index = strlen(buildings)-2; 
            }else{
              char_index = char_index-2; //go backward
            }
          }
        }
        scroll_timer = millis();
      }
      char current_letter[50]={buildings[char_index],buildings[char_index+1]}; //again gets value at char_index
      strcpy(output,query_string);
      strcat(output,current_letter);
      if(button==2){ //
        state=2;
        strcpy(output,"");
      }
    }else if(state==2){
      strcpy(msg,"calculating..");
      strcpy(output,msg);
      state=3;
    }else if(state==3){

      Serial.printf("**********%s\n",query_string);
      sprintf(q,"%s",query_string);

      lookup(q,msg);
      strcpy(output,msg);
      strcpy(query_string,"");
      strcpy(encrypted_query_string,"");
      state=0;
    }
    else if(state==4){
        if(button==1){
        // char current_letter[50] = {rooms[char_index][char_index2],rooms[char_index][char_index2+1],rooms[char_index][char_index2+2] }; //gets the value at char_index
        char current_letter[50] = {floors[char_index2]};
        strcat(query_string,current_letter);
        strcpy(output,query_string);
        char_index2 =0;
        state = 2;
      }else if(millis() - scroll_timer >= scroll_threshold){
        if (abs(angle) > angle_threshold){
          if(angle>0){
            if(char_index2 == (strlen(floors[char_index])-1)){
              char_index2 = 0;
            }else{
              char_index2 = char_index2 +1; // go forward 
            }
          }
          if(angle<0){
            if(char_index2==0){
              char_index2 = strlen(floors[char_index])-1; 
            }else{
              char_index2 = char_index2-1; //go backward
            }
          }
        }
        scroll_timer = millis();
      }
      // char current_letter[50]={alphabet2[char_index]}; //again gets value at char_index
      // char current_letter[50] = {rooms[char_index][char_index2],rooms[char_index][char_index2+1],rooms[char_index][char_index2+2] }; //gets the value at char_index
      char current_letter[50] = {floors[char_index2]};
      strcat(output,current_letter);
      if(button==2){ //
        state=2;
        strcpy(output,"");
      }
    }
};

void DestinationSelection::display(){
    // tft.fillScreen(TFT_BLACK);
    // tft.setCursor(0, 0, 1);
    // tft.println(output);
};

char* DestinationSelection::get_destination() {
    return destination;
};

int DestinationSelection::get_destination_floor() {
    return destination_floor;
};