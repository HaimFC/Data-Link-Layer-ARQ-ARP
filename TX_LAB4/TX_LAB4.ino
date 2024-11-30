#include <C:\Users\HaimT\Desktop\LAB4_NEW\EthernetLab.h>
//------------------------------------------Defines------------------------------------
#define newData "Shahaf & Haim"                     //Data in string                   ||-------------------------------------------------------------||
#define SIZE 8                                      //Data size                        || Able to change any data size between 0-12 and any duty cycle|| 
#define BIT_TIME 10                                 //duty cycle in millis             ||-------------------------------------------------------------||
#define IFG_TIME BIT_TIME*12*8                      //Time when we know we finished to recieve
#define RTT_TIME 2*IFG_TIME                         //Initial RTT time
#define outputDataPin 5                             //output Pin
#define inputDataPin 6                              //Input pin
#define outputClkPin 7                              //output Pin
#define inputClkPin 8                               //Input pin

//------------------------------------------Structs-------------------------------------
typedef struct Frame {                              //Frame Struct
  uint8_t destination_address = 30;
  uint8_t source_address = 20;
  uint8_t frame_type = 0;
  uint8_t length;
  uint8_t* payload;
  uint32_t crc;
};
typedef struct Ack {                                //Ack Struck
  uint8_t destination_address;
  uint8_t source_address;
  uint8_t frame_type;
  uint8_t RN;
};
//------------------------------------LAYER 1 TX----------------------------------------
uint8_t array2send[100];                            //the payload as int
int SN = 0;                                         //SN counter
int frameIndex = 0;                                 //bytes index of the frame
boolean clkVal = false;                             //clk value
unsigned int dataToSend = 0;                        //initialize data to send
bool startTosend = false;                           //flag to check if we start to send the frame
//------------------------------------LAYER 1 RX----------------------------------------
uint8_t arrayRecieved[4];                           //the ack as int array
int ackIndex = 0;                                   //index to run on the ack array
int packSize = 0;                                   //keep the size of the whole pack (not only payload)
unsigned int recievedData=0;                        //initialize data to recieve
int index = 0;                                      //index of the sent data
int count = 0;                                      //size of the recieved data
unsigned long RTT = RTT_TIME;                       //keep the Avarage RTT TIME for ACK packs
bool flagIfRead = false;                            //flag to check if digit is already read
bool startTosimple = false;                         //flag to check if we start to sample the ack
//------------------------------------LAYER 2 TX----------------------------------------
char dataTX2[100] = newData;                        //the payload as char
Frame dataFrame;                                    //whole data as frame to send
int indexOfFrame = 0;                               //index to run on the int payload array
int frameNum = 0;                                   //counter for the number of frames we sent (only for feeling)
//------------------------------------LAYER 2 RX----------------------------------------
Ack ackFrame;                                       //whole ack packet we recieved
//------------------------------------Time Samples--------------------------------------
unsigned long current = 0;                          //keep the current time
unsigned long currentIFG = 0;                       //time sample for the IFG
unsigned long currentIFG_RTT = 0;                   //time sample for the RTT
//------------------------------------LAYERS FLAGS--------------------------------------
bool layer_2_tx_request = false;                    //flag who represent when layer 2 tx is need service from layer 1 tx
bool layer_1_tx_busy = false;                       //flag who represent when layer 1 tx busy
bool layer_1_tx_request = false;                    //flag who represent when layer 1 tx is need service from layer 1 rx
bool layer_1_rx_busy = false;                       //flag who represent when layer 1 rx busy
bool layer_1_rx_request = false;                    //flag who represent when layer 1 rx is need service from layer 2 rx
bool layer_2_rx_value = false;                      //value if the packet is good or not
//---------------------------------------------------------------------------------------

void setup() {
    setMode(0);                                      //CRC no mistakes mode - TX MODE
    Serial.begin(115200);                            //BAUD_RATE = 115200 
    pinMode(outputDataPin, OUTPUT);                  //initialize Tx - analog pin
    pinMode(inputDataPin,INPUT);                     //initialize Rx - analog pin
    pinMode(outputClkPin, OUTPUT);                   //initialize clkIn 
    pinMode(inputClkPin,INPUT);                      //initialize clkOut 
}

void loop() {
    layer2_tx();                                     //Layer 2 - TX Function - create frames
    layer1_tx();                                     //Layer 1  - TX Function - Send frames
    layer1_rx();                                     //Layer 1  - RX Function - Recieve ACK 
    layer2_rx();                                     //Layer 2  - RX Function - Check ACK 
}
void  layer2_tx()
{     
      if(layer_1_tx_busy || layer_1_rx_busy)        //The default layer, check if the other layers are in middle of procces                  
          return;
      long timer = millis();                        //take current time(since arduino begins)
      if(layer_2_rx_value)                          //check if the packet value flag is correct
        frameNum++;                                 //if does, increase the counter
      if (timer - currentIFG >= IFG_TIME || !startTosend)                
      {                                             //when IFG time is passed
        startTosend = true;                         //flag that we start to send
        Serial.print("------------Frame no.");      //print frame number
        Serial.print(frameNum);
        Serial.println("-----------------");

        Serial.print("The data  in this frame is: ");//print frame data
        Serial.println(newData);
        
        dataFrame.payload = dataTX2;                //fill the frame struct fields
        dataFrame.length = strlen(dataTX2);         
        createFrame();                              //create the frame
        packSize = indexOfFrame+1;                  //the size is index + 1 (index star with 0)
        indexOfFrame=0;                             //nullify for the next frame
        layer_2_tx_request = true;                  //Ask request from next layer
        
      }
}
void layer1_tx()
{ 
    dataToSend = array2send[frameIndex];            //sends Bytes 
    if(layer_2_tx_request)                          //check if the layer above ask for service
      {
        layer_2_tx_request = false;                 //turn of the flag
        layer_1_tx_busy = true;                     //now this layer is busy
      }
    if(!layer_1_tx_busy)                            //check if the layer in remaining procces
        return;
    //Serial.println("layer1_tx()");  
    if(index >= SIZE)                               //check that max digits sent is by SIZE
    {
      index = 0;
      Serial.print("Byte Number [");                // print the bytes by order
      Serial.print(frameIndex);
      Serial.print("] is: ");  
      Serial.println(dataToSend);                   
      frameIndex++;
    }
  
    boolean leastSign = bitRead(dataToSend,index);  //lsb bit
    long timer = millis();                          //time since turn on
    
    if (timer - current >= BIT_TIME)                //Timer
    {
      clkVal = !clkVal;                             //flip the 1->0 and 0->1
        
        if(clkVal)                                  //if bit is 1 send data
        {
            if(frameIndex == packSize )             //when we finish to send last BYTE
            {
              layer_1_tx_busy = false;              // this layer finish the job
              layer_1_tx_request = true;            //ask request from next layer
              frameIndex =0;                        // initialize frame index
              currentIFG = millis();                //sample the IFG time
              currentIFG_RTT = millis();            //samaple RTT time
              return;
            }     
            if(leastSign)                           //check lsb
                digitalWrite(outputDataPin, HIGH);  //if 1 so data pin 5 is 5 volt
            else
                digitalWrite(outputDataPin, LOW);   //if 0 so data pin 5 is 0 volt
            index++;                                //index of the data
            digitalWrite(outputClkPin, LOW);
        }
        else
            digitalWrite(outputClkPin, HIGH);       //if bit is 0 send clk 
        
      current = millis();                           //current time                        
    }
}
void layer1_rx()
{
  long timer = millis();                               //sample time since arduino turned on
  if(layer_1_tx_request)                               //check if layer above need service
  {
      layer_1_tx_request = false;                      //turn off the service flag
      layer_1_rx_busy = true;                          //this layer now busy
  }
  if(!layer_1_rx_busy)                                 //check if the layer is in middle of procces
    return;
  if (!digitalRead(inputClkPin))                       //check if we are in "not read" mode
  {
    //Serial.println("case 1"); 
    flagIfRead = 0;                                    //if does turn flag to zero
  }
  else if(digitalRead(inputClkPin) && flagIfRead == 0) //check if we are in "read" mode and flag is zero
  {     
    startTosimple = true; 
    layer_1_rx_busy = true;
    flagIfRead = 1;                                    //turn flag to 1- we already read
    bitWrite(recievedData, count, digitalRead(inputDataPin));   //turn the digits into one number
    count++;                                           //count size of the nuber
  }
  else
  {
   if (timer - currentIFG_RTT >= RTT && !startTosimple)//in case of nothing recieved check if RTT TIME is passed
    {
        arrayRecieved[0] = 0;                          //nullify ACK 
        arrayRecieved[1] = 0;
        arrayRecieved[2] = 0;
        arrayRecieved[3] = 0;
        layer_1_rx_busy = false;                      //layer is not busy anymore
    }
  }
  if (count == SIZE)                                  //check that all bits from the current byte are recieved
  {
    arrayRecieved[ackIndex++] = recievedData;         //turn the byte in the array
    if(ackIndex == 4)                                 //if finihsed recieve ACK
     {
        Serial.print("The ACK pack is: [");       
        Serial.print(arrayRecieved[0]);
        Serial.print(", ");
        Serial.print(arrayRecieved[1]);
        Serial.print(", ");
        Serial.print(arrayRecieved[2]);
        Serial.print(", ");
        Serial.print(arrayRecieved[3]);
        Serial.println("]"); 
        layer_1_rx_busy = false;                      //layer is not busy anymore
        layer_1_rx_request = true;                    //ask request from next layer
        ackIndex = 0;                                 //nullify ACK index
        startTosimple = false;                        //nullify the flag who remember we start to sample
     }
    //Serial.println(recievedData,BIN);               //print the final data
    count = 0;                                        //reset counter for new data
    recievedData = 0;                                 //reset data for new data
  }
}
void layer2_rx()
{
  long timer = millis();  
  if(!layer_1_rx_request)                             //check if the layer above ask for service
    return;
  layer_1_rx_request = false;                         //if service is required turn off the flag
  ackFrame.destination_address = arrayRecieved[0];    //create ACK 
  ackFrame.source_address = arrayRecieved[1];
  ackFrame.frame_type = arrayRecieved[2];
  ackFrame.RN = arrayRecieved[3];
  if(ackFrame.destination_address == 20 && ackFrame.source_address == 30) 
  {                                                   //check if the ACK source and destination are correct
    if(ackFrame.frame_type == 1 && (SN + 1)%2 == ackFrame.RN)
    {                                                 //check SN and Type(that this is ACK)
        RTT = ((RTT + (timer - currentIFG_RTT))/2);   //for correct ACK we will recalc RTT
        Serial.print("The RTT is: ");
        Serial.println(RTT);
        SN++;                                         //increase SN
        layer_2_rx_value = true;                      //turn the flag that the pack is correct
        return;                                     
    }
  }
  layer_2_rx_value = false;                           //in any other case we assume the package is corrupt
}

void createFrame()
{
    for(int i = 0; i < 4; i++){                               //insert the default fields of the frame struct
        array2send[i] = *((uint8_t *)&dataFrame + i);
        //Serial.println(array2send[i]);
    }
    for(int i = 0; i < strlen(dataFrame.payload); i++){       //insert payload to the array
      array2send[i+4] = dataFrame.payload[i];
      indexOfFrame = i+4;
    } 
    array2send[++indexOfFrame] = SN%2;                        //insert SN to the array
    dataFrame.crc = calculateCRC(array2send, indexOfFrame+1); //create CRC
    uint8_t * convCRC = (uint8_t *)&dataFrame.crc;            //insert CRC   
    array2send[++indexOfFrame] = convCRC[3];
    array2send[++indexOfFrame] = convCRC[2];
    array2send[++indexOfFrame] = convCRC[1];
    array2send[++indexOfFrame] = convCRC[0];
}
