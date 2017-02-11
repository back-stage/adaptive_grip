
// Pin defs
#define STP 2 // All motor steps are tied together
#define DIR_1 3 // Each motor has its own direction pin
#define DIR_2 4
#define DIR_3 5
#define DIR_4 6
#define DIR_5 7
#define DIR_6 8
#define EN 10
#define ARRAY_SIZE 6
#define FORCE_THRESHOLD 500

#define FORCE_THRESHOLD 900

//Declare variables for functions
char userInput;
boolean firstRun = true;

//Arrays
int 		motors[ARRAY_SIZE] = {1,2,3,4,5,6}; //rotate motor or vibrate in place

// True = open
// False = close
boolean 	directions[ARRAY_SIZE] = {false,false,false,true,true,true};//clockwise or counter clockwise

// Counter to remember how many steps each motor's moved
int 		positions[ARRAY_SIZE] = {0,0,0,0,0,0}; //with respect to 'home' position

// Bounds of the steps - 0 is the lower limit for all
int 		limits[ARRAY_SIZE] = {950,850,800,650,600,740};

// "Curl Limits"
int 		curlLimits[ARRAY_SIZE] = {400,400,400,50,50,50};
int     clampLimits[ARRAY_SIZE] = {400,400,400,450,450,450};

int force[3] = {0, 0, 0};
//force[0]: paired with motor 1 and 6, A2
//force[1]: paired with motor 2 and 4, A0
//force[2]: paired with motor 3 and 5. A1


// function declarations
void printArrays(int(*motors), boolean directions[], int arrayLength);
void zeroIntArray(int (*motors), int arrayLength);
void falseBooleanArray(boolean directions[], int arrayLength);
void printPositions(int (*positions));
void userMotorChoice(char userInput, int (*motors), boolean directions[]);
void displayMenu();
void driveMotor(int (*motors), int degRotation, boolean directions[], int (*positions), int arrayLength );
void selectMotor(int (*motors), boolean directions[], int (*positions));
void setPins();
void holdPosition(int mSecs);
void testAllMotors(int (*motors), boolean directions[], int (*positions));
void setMotors(int (*index), int (*motors));
void setOriginalPosition(int enablePin, int (*positions));
void openFinger1(int (*motors), boolean directions[], int (*positions));
void openFinger3(int (*motors), boolean directions[], int (*positions));
void openAllJoints(int (*motors), boolean directions[], int (*positions), int(*limits));
void closeAllJoints(int (*motors), boolean directions[], int (*positions));
void curlGrab(int(*motors),boolean directions[], int(*positions), int (*limits), int);

void setup() { //--------------------------------------------------- SETUP -----------------------------------------------------------------//
  pinMode(STP,OUTPUT);
  pinMode(DIR_1,OUTPUT);
  pinMode(DIR_2,OUTPUT);
  pinMode(DIR_3,OUTPUT);
  pinMode(DIR_4,OUTPUT);
  pinMode(DIR_5,OUTPUT);
  pinMode(DIR_6,OUTPUT);
  pinMode(EN,OUTPUT);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  
  setPins(); 
  Serial.begin(9600); 
  displayMenu();
}

void loop() { // ------------------------------------------------------- MAIN LOOP --------------------------------------------------//
  if (firstRun){
    setOriginalPosition(EN, positions);
    firstRun = false;  
  }
  
  while (Serial.available()) {
    userInput = Serial.read();
    if (userInput =='0'){
      printPositions(positions);
      displayMenu();
    }
    else{

      zeroIntArray(motors,ARRAY_SIZE);
      userMotorChoice(userInput, motors, directions, force); // Sets new motors array and directions array
      driveMotor(motors, 5, directions, positions, ARRAY_SIZE);

      //debugging
//      for(int i = 0; i < 6; i++){
//        Serial.print("Position of motor ");
//        Serial.print(i+1);
//        Serial.print(" is: ");
//        Serial.println(positions[i]);
//      }
      //displayMenu();
    } 
  }
}









