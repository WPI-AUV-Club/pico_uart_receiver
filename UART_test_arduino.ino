int incomingByte = 0; // for incoming serial data
unsigned long startTime = millis();
unsigned long currTime = millis();
unsigned long lastReceivedPacket = millis();
int inc = 0;
bool lostComms = false;
String prevBootIndex = "";
String incomingPacket = "";

void setup() {
  //8 bit data frame, even parity SERIAL_8E1
  Serial.begin(38400); //initialize serial communication at a 9600 baud rate
  Serial1.begin(38400, SERIAL_8E1); //initialize serial communication at a 9600 baud rate
}

void loop() {
  currTime = millis();


  //Send a command packet every 20ms (50hz)
  if (currTime-startTime > 20) {
    startTime = currTime;

    String packet = "";
    inc = inc+1;
    if (inc > 254) inc = 0;

    for(int x = 0; x < 8; x++) {
      int led_pwm = (int)(inc+(255/8.0)*x-1) % 254;
      led_pwm = max(0, led_pwm);
      led_pwm = min(254, led_pwm);
      packet += (char)led_pwm;
    }

    // packet += (char)((int)inc);
    // packet += (char)0;
    // packet += (char)254;
    // packet += (char)100;
    // packet += (char)200;
    // packet += (char)127;
    // packet += (char)127;
    // packet += (char)127;

    packet += (char)255;

    if (!lostComms) {
      Serial1.print(packet);
    }
  }
    
  //TODO: Flag non-sequentially indexed ACK packets
  //Read incoming packets
  //Example packet format: "AB-MSG=ACK:A"
  //Want to print first two char (boot counter header) as decmial values
  //Also would be nice to print the ACK increment as a decimal value
  //Null terminator seperates packets and should appear nowhere else in code

  if (Serial1.available() > 0) {
    incomingByte = Serial1.read();
    // Serial.print((char)incomingByte);
  // if (Serial1.available() > 0) {
  //   incomingByte = Serial1.read();

    if ((char)incomingByte == '\0') {
      Serial.println(incomingPacket);
      lostComms = false;
      incomingPacket = "";
      lastReceivedPacket = currTime;
    } else if (incomingPacket.length() < 6) {
      char buffer[4];
      sprintf(buffer, "%03d", incomingByte); 
      incomingPacket += buffer;
    } else if (incomingPacket.indexOf("ACK:") > 0) {
      incomingPacket += incomingByte;
    } else {
      if (incomingPacket.length() == 6 && prevBootIndex != incomingPacket) {
        Serial.println("RECEIVER RESET DETECTED");
        delay(500);
        prevBootIndex = incomingPacket;
      }
      incomingPacket += (char)incomingByte;
    }
  }


  //Throw error if slave device stops responding
  if (currTime - lastReceivedPacket > 80) {
    lostComms = true;
    Serial.println("SELF-ERR=NO_MSG_RECEIVED");
    lastReceivedPacket = currTime; //Prevent from flooding serial at full loop speed
  }
}