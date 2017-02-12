void printArrays(int(*motors), boolean directions[], int arrayLength){
  for (int x = 0; x < arrayLength; x++){
    Serial.print("motors[" + String(x) + "] = " + String(motors[x]) + " | ");
  }
  Serial.println();
  for (int x = 0; x < arrayLength; x++){
    Serial.print("directions[" + String(x) + "] = " + String(directions[x]) + " | ");
  }
  Serial.println();
}

// -----------------------------------Reset Array ----------------------------//
void zeroIntArray(int (*motors), int arrayLength)
{
  for (int x = 0; x < arrayLength; x ++){
    motors[x] = 0;
  } 
}

//------------------------------------Reset Boolean Array --------------------//
void falseBooleanArray(boolean directions[], int arrayLength)
{
  for (int x = 0; x < arrayLength; x++){
    directions[x] = false;
  }
}

void printPositions(int *positions) {

  for (int x = 0; x < 6; x++) {
    Serial.print("position[" + String(x) + "] :" + String(positions[x]) + "   |   ");
  }
  Serial.println("");
}

void userMotorChoice(char userInput, int (*motors), boolean directions[], int (*force))
{
  switch (userInput) {
    case '1': {
        motors[0] = DIR_1;
        directions[0] = true;
        break;
    } case '2': {
        motors[1] = DIR_2;
        directions[1] = true;
        break;
    } case '3': {
        motors[2] = DIR_3;
        directions[2] = true;
        break;
    } case '4': {
        motors[3] = DIR_4;
        directions[3] = true;
        break;
    } case '5': {
        motors[4] = DIR_5;
        directions[4] = true;
        break;
    } case '6': {
        motors[5] = DIR_6;
        directions[5] = true;
        break;
    } case '7': {
        motors[0] = DIR_1;
        directions[0] = false;
        break;
    } case '8': {
        motors[1] = DIR_2;
        directions[1] = false;
        break;
    } case 'a': {
        motors[2] = DIR_3;
        directions[2] = false;
        break;
    } case 'b': {
        motors[3] = DIR_4;
        directions[3] = false;
        break;
    } case 'c': {
        motors[4] = DIR_5;
        directions[4] = false;
        break;
    } case 'd': {
        motors[5] = DIR_6;
        directions[5] = false;
        break;
    } case 'e': {
        openAllJoints(motors,directions,positions,limits);
        curlGrab(motors, directions, positions, curlLimits, force);
        break;
    } case 'f':{
        closeAllJoints(motors,directions,positions);
        break;
    } case 'h': {
        openAllJoints(motors,directions,positions,limits);
        break;
    } case 'i': {
        openAllJoints(motors,directions,positions,limits);
        curlGrab(motors,directions,positions,clampLimits,force);
        break;
    } case 'j': {
        grab(motors, directions,positions,innerLimits, outerLimits);
        break;
    } 

    case 'z': {
        firstRun = true;
        setPins();
        break;
    } case 'k': {
        checkPressure(force);
        Serial.println(force[0]);
        Serial.println(force[0]);
        Serial.println(force[0]);
    }default: {
        //do something
        Serial.println("ERROR, User input was incorrect");
        directions[0] = false;
        break;
      }
  }
}

void checkPressure(int (*force)){
  force[0] = analogRead(A2);//1-6
  force[1] = analogRead(A0);//2-4
  force[2] = analogRead(A1);//3-5
}



void displayMenu(){
  Serial.println("Enter number for control option:");
  Serial.println("0. Print Positions");
  Serial.println("1. Motor 1, Clockwise");
  Serial.println("2. Motor 2, Clockwise");
  Serial.println("3. Motor 3, Clockwise");
  Serial.println("4. Motor 4, Clockwise");
  Serial.println("5. Motor 5, Clockwise");
  Serial.println("6. Motor 6, Clockwise");
  Serial.println("7. Motor 1, Counter-Clockwise");
  Serial.println("8. Motor 2, Counter-Clockwise");
  Serial.println("a. Motor 3, Counter-Clockwise");
  Serial.println("b. Motor 4, Counter-Clockwise");
  Serial.println("c. Motor 5, Counter-Clockwise");
  Serial.println("d. Motor 6, Counter-Clockwise");
  Serial.println("e. Curl Grab");
  Serial.println("z. Reset positions, claw limp");
  Serial.println("f. Close All Joints");
  Serial.println("h. Open All Joints");
  Serial.println("i. Grab Object");
  Serial.println("k. Show Pressure Values");
  Serial.println();
}
