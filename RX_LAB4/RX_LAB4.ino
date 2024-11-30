#include <C:\Users\HaimT\Desktop\LAB4_NEW\EthernetLab.h>
//------------------------------------------Defines-------------------------------------
#define DATA 0b11111111                             //Data to send                      ||-------------------------------------------------------------||
#define SIZE 8                                      //Data size                         || Able to change any data size between 0-12 and any duty cycle|| 
#define BIT_TIME 10                                 //1/100 clk duty cycle              ||-------------------------------------------------------------||
#define IFG_TIME BIT_TIME*12*8                      //IFG TIME - Half of the time represent finish recieve data
#define outputDataPin 5                             //output Pin
#define inputDataPin 6                              //Input pin
#define outputClkPin 7                              //output Pin
#define inputClkPin 8                               //Input pin
//------------------------------------------Structs-------------------------------------
typedef struct Frame {                               //Frame Struct
  uint8_t destination_address;
  uint8_t source_address;
  uint8_t frame_type;
  uint8_t length;
  uint8_t* payload;
  uint8_t  SN;
  uint32_t crc;
};
typedef struct Ack {                                 //Ack Struck
  uint8_t destination_address = 20;
  uint8_t source_address = 30;
  uint8_t frame_type = 1;
  uint8_t RN = 0;
};
//-----------------------------------------Layer 1 TX-----------------------------------
boolean clkVal = false;                             //clk value
unsigned int dataToSend = DATA;                     //initialize data to send
int index = 0;                                      //index of the sent data
int ackIndex = 0;                                   //index of the ACK array
bool startTosimple = false;                         //flag to check if start to sample
//-----------------------------------------Layer 1 RX-----------------------------------
unsigned int recievedData=0;                        //initialize data to recieve
int count = 0;                                      //size of the recieved data
bool flagIfRead = false;                            //flag to check if digit is already read
uint8_t arrayRecieved[100];                         //array to keep recieved data
//-----------------------------------------Layer 2 TX-----------------------------------
Frame dataFrame;                                    //frame recieved 
uint8_t array2send[4];                              //array keep the ACK information
//-----------------------------------------Layer 2 RX-----------------------------------
Ack ackFrame;                                       //ACK to send
int indexOfFrame = 0;                               //index of the frame who recieved
String reason = "";
//-----------------------------------------TIME-----------------------------------------
unsigned long current = 0;                          //keep the current time
unsigned long currentIFG = 0;                       //keep the sampled IFG time
//-----------------------------------------LAYERS FLAGS---------------------------------
bool layer_1_rx_busy = false;                       //flag who represent that layer 1 rx is busy
bool layer_1_rx_request = false;                    //flag who represent that layer 1 rx is need service
bool layer_2_rx_request = false;                    //flag who represent that layer 2 rx is need service
bool layer_2_tx_request = false;                    //flag who represent that layer 2 tx is need service
bool layer_1_tx_busy = false;                       //flag who represent that layer 1 tx is busy
//-----------------------------------------ERROR PROB-----------------------------------
int numOfBadFrames = 0;                             //count number of corrupt frames
int numOfFrames = 0;                                //count number of frames in total(correct + incorrect)
//--------------------------------------------------------------------------------------

void setup() {
    setMode(1);
    Serial.begin(115200);                           //BAUD_RATE = 115200 
    pinMode(outputDataPin, OUTPUT);                 //initialize Tx - analog pin
    pinMode(inputDataPin,INPUT);                    //initialize Rx - analog pin
    pinMode(outputClkPin, OUTPUT);                  //initialize clkIn 
    pinMode(inputClkPin,INPUT);                     //initialize clkOut 
}

void loop() {
    layer1_rx();                                    //Layer 1 reciever function
    layer2_rx();                                    //Layer 2 reciever function
    layer2_tx();                                    //Layer 2 transfer function
    layer1_tx();                                    //Layer 1 transfer function
}

void layer2_tx()
{
  if(!layer_2_rx_request)                           //check if layer 2 rx is asking for service
      return;
  Serial.println("----------Summary----------");    //print the details and error probability
  Serial.print("Num Of Frames: ");
  Serial.println(numOfFrames);
  Serial.print("Num Of Bad Frames: ");
  Serial.println(numOfBadFrames);
  Serial.print("Error probability: ");
  if(numOfFrames == 0)
    Serial.println(0);
  else
    Serial.println((double)numOfBadFrames/numOfFrames);
  Serial.println("---------------------------");      
  layer_2_rx_request = false;                       //turn off layer 2 service request
  array2send[0] = ackFrame.destination_address;     //create ACK
  array2send[1] = ackFrame.source_address;
  array2send[2] = ackFrame.frame_type;
  array2send[3] = (ackFrame.RN)%2;
  layer_2_tx_request = true;                        //ask request from layer 1 tx and send the ACK
}
void layer2_rx()
{
  if(!layer_1_rx_request)                           //check if layer 1 rx is asking for service
    return;
  layer_1_rx_request = false;                       //turn off the service flag
  dataFrame.destination_address = arrayRecieved[0]; //create frame struct with the details we recieved
  dataFrame.source_address = arrayRecieved[1];
  dataFrame.frame_type = arrayRecieved[2];
  dataFrame.length = arrayRecieved[3];
  dataFrame.payload = arrayRecieved + 4;
  dataFrame.SN = arrayRecieved[4 + dataFrame.length];
  dataFrame.crc = (uint32_t)arrayRecieved[dataFrame.length + 5]<<24| (uint32_t)arrayRecieved[dataFrame.length + 6]<<16 | (uint32_t)arrayRecieved[dataFrame.length + 7]<<8 | (arrayRecieved[dataFrame.length + 8]);
  String fullData = (char*)dataFrame.payload;       //create string of the payload
  fullData.remove(dataFrame.length,5);              //remove SN and CRC from the string 
                                                    //calc the CRC
  uint32_t convCRC = calculateCRC(arrayRecieved, dataFrame.length+5);
  numOfFrames++;                                    //increase the number of the total frames we recieved                                                    
  if(dataFrame.destination_address == 30 && dataFrame.source_address == 20)
{                                                   //check if the source and destination are correct
    if(dataFrame.frame_type == 0 && dataFrame.crc == convCRC)
    {                                               //check if the type and the crc are correct
      if(dataFrame.SN == (ackFrame.RN)%2)           //check if the SN is correct
      {                                             //if does print the data
        Serial.print("The frame is correct, the data is: ");
        Serial.println(fullData);
        ackFrame.RN++;                              //increase SN
        layer_2_rx_request = true;                  //ask for service from layer 2 tx
        return;
      }
      else
        reason = "wrong serial number";             //keep the incorrect reason
    }
    else
       reason = "wrong CRC";                        //keep the incorrect reason
  }
  else
    reason = "wrong destination or source";         //keep the incorrect reason
  Serial.print("Incorrect frame - ");               //if any of the details is wrong
  Serial.println(reason);
  numOfBadFrames++;                                 //increase the number of corrupted frames
  layer_2_rx_request = true;                        //ask for service from layer 2 tx (Without SN increase)
  reason = "";
}   
void layer1_tx()
{ 
    if(layer_2_tx_request)                          //check if layer 2 tx is asking for service
    {
      layer_2_tx_request = false;                   //turn off the service flag
      layer_1_tx_busy = true;                       //this layer is starting process
    }
    if(!layer_1_tx_busy)                            //check if the layer is in middle of process
      return;
    if(index >= SIZE)                               //check that max digits sent is by SIZE
    {
      index = 0;                                    //nullify index
      ackIndex++;                                   //increase ACK index
    }
    dataToSend = array2send[ackIndex];              //send the next byte
    boolean leastSign = bitRead(dataToSend,index);  //lsb bit
    long timer = millis();                          //time since turn on
    if (timer - current >= BIT_TIME)                //Timer
    {
      clkVal = !clkVal;                             //flip the 1->0 and 0->1
        if(clkVal)                                  //if bit is 1 send data
        {
            if(ackIndex == 4)                       //if sent 4 bytes (ACK SIZE)
            {
                digitalWrite(outputClkPin, LOW);    //stop sending
               layer_1_tx_busy = false;             //finished process
               ackIndex = 0;                        //nullify ACK index
            }                     
            digitalWrite(outputClkPin, LOW);        //do not send clk
            if(leastSign)                           //check lsb
                digitalWrite(outputDataPin, HIGH);  //if 1 so datapin 5 is 5 volt
            else
                digitalWrite(outputDataPin, LOW);   //if 0 so datapin 5 is 0 volt
            index++;                                //index of the data
        }
        else
            digitalWrite(outputClkPin, HIGH);       //if bit is 0 send clk      
      current = millis();                           //current time                        
    }
}
void layer1_rx()
{
  long timer = millis();                                        //sampling the time since arduino turn on
  if (!digitalRead(inputClkPin))                                //check if we are in "not read" mode
    flagIfRead = 0;                                             //if does turn flag to zero                                                 
  else if(digitalRead(inputClkPin) && flagIfRead == 0)          //check if we are in "read" mode and flag is zero
  {
    if(!startTosimple)
      Serial.println("Recieving new frame"); 
    startTosimple = true;                                       //flag who represent if we alreadt start to recieve data
    layer_1_rx_busy = true;                                     //this layer is starting process
    flagIfRead = 1;                                             //turn flag to 1- we already read
    bitWrite(recievedData, count, digitalRead(inputDataPin));   //turn the digits into one number
    count++;                                                    //increase count(size of the recieve data in bits)
    currentIFG = millis();                                      //sample the last time we recieved bit
  }
  else
  {
  if (timer - currentIFG >= IFG_TIME*0.5 && startTosimple)      //check if IFG*0.5 time is passed so its mean no bits are about to recieve
    {
      startTosimple =false;                                     //nullify for the nest frame
      layer_1_rx_busy = false;                                  //finished the process
      layer_1_rx_request = true;                                //asking for service from the next layer
      indexOfFrame = 0;                                         //nullify for next frame recieve
    }  
  }
  if (count == SIZE)                                            //check that all bits recieved
  {
    arrayRecieved[indexOfFrame++] = recievedData;               //increase the frame index for getting next byte
    Serial.println(recievedData,DEC);                           //print the final data
    count = 0;                                                  //reset counter for new data
    recievedData = 0;                                           //nullify the last byte we recived
  }
  
}
