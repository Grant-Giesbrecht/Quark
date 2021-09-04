#define DATA_BUFFER_SIZE 512
#define LOAD_BUFFER_SIZE 30

long addr_buf[DATA_BUFFER_SIZE];
int data_buf[DATA_BUFFER_SIZE];

int buffer[LOAD_BUFFER_SIZE];

//Set load index to 0
size_t main_load_idx = 0;
int load_idx = 0; //idx to load buffer (ie serial buffer, not main data array)
String error = "";
size_t file_len_max = 32768;

int bus_pins_out[16] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 22, 23, 24, 25}; // From 0-15
int bus_pins_in[8] = {38, 39, 40, 41, 42, 43, 44, 45}; //From 0-7
unsigned int write_time_74hc574 = 1000; //in microseconds, time to keep 574's CP active
unsigned int trigger_operation_time = 1000; //in microseconds, time to keep trigger operation line (to MCU) active
unsigned int write_time_delay = 1000; //in microseconds
unsigned int read_time_delay = 1000; //in microseconds, time for data to be correct and stable on data bus after operation triggered
unsigned int clear_pause_time = 1000000; //in microseconds, time for clear pause pin to be active

bool erase_before_write = false;

// FLash pins - OUTPUT
#define PIN_LD_ADDR_S0 31
#define PIN_LD_ADDR_S1 32
#define PIN_LD_DATA_S0 33
#define PIN_LD_DATA_S1 34
#define PIN_OPCODE0 35
#define PIN_OPCODE1 36
#define PIN_CPU_WR_EN_S0 29
#define PIN_CPU_WR_EN_S1 30
#define PIN_BUF_OE_S0 37
#define PIN_BUF_OE_S1 17
#define PIN_TRIG_S0 16
#define PIN_TRIG_S1 15
#define PIN_WRADDR_BUS 26
#define PIN_WRDATA_BUS 27

//Flash pins - INPUT
#define PIN_ACTIVE_S0 A0
#define PIN_ACTIVE_S1 A1
#define PIN_PROTECTED_S0 A2
#define PIN_PROTECTED_S1 A3

//RAM pins - OUTPUT
#define PIN_RAM_LD_ADDR_SR0 20
#define PIN_RAM_WR_SR0 21
#define PIN_RAM_RD_SR0 18

//System state pins - INPUT
#define PIN_USB_PROG A4
#define PIN_SYSTEM_PROGRAM 14 //TODO: Have code check that the machine is in program mode

// #define PIN_TRIG 9 //Pin to trigger operation
// #define PIN_DATA 6 //Pin to clock data buffer
// // #define PIN_MAR0 5 //Pin to clock MAR0
// // #define PIN_MAR8 4 //Pin to clock MAR8
// #define PIN_OPCODE0 11 //Pin to set OpCode bit 0
// #define PIN_OPCODE1 10 //Pin to set OpCode bit 1
// #define PIN_CLEAR_PAUSE 8 //Pin to clear pause from read operation
// #define PIN_CHIP_ENABLE 7//Pin to control chip enable
// #define PIN_A16 12 //Pin to control 17th address line

bool run_write_cycle = false;
size_t write_cycle_end = DATA_BUFFER_SIZE;

char charbuf[120];
char charbuftemp[20];

size_t req_read_len = 0;
bool run_read_cycle = false;
size_t next_read_idx = 0;
size_t read_cycle_end = DATA_BUFFER_SIZE;

bool in_write_mode = false;

void setup() {
    Serial.begin(115200);

	// Initialize bus pins
	for (int i = 0  ; i  < 16 ; i++){
		pinMode(bus_pins_out[i], OUTPUT);
        digitalWrite(bus_pins_out[i], LOW);
	}
    for (int i = 0  ; i  < 8 ; i++){
        pinMode(bus_pins_in[i], INPUT);
    }

	// Bus buffer control pins
	pinMode(PIN_WRDATA_BUS, OUTPUT);
	pinMode(PIN_WRDATA_BUS, OUTPUT);
	digitalWrite(PIN_WRADDR_BUS, LOW);
	digitalWrite(PIN_WRDATA_BUS, LOW);

	// Flash write address buffer pins
	pinMode(PIN_LD_ADDR_S0, OUTPUT);
	pinMode(PIN_LD_ADDR_S1, OUTPUT);
	digitalWrite(PIN_LD_ADDR_S0, LOW);
	digitalWrite(PIN_LD_ADDR_S1, LOW);

	// Flash write data buffer pins
	pinMode(PIN_LD_DATA_S0, OUTPUT);
	pinMode(PIN_LD_DATA_S1, OUTPUT);
	digitalWrite(PIN_LD_DATA_S0, LOW);
	digitalWrite(PIN_LD_DATA_S1, LOW);

	// Op. Code Pins`
	pinMode(PIN_OPCODE0, OUTPUT);
	pinMode(PIN_OPCODE1, OUTPUT);
	digitalWrite(PIN_OPCODE0, LOW);
	digitalWrite(PIN_OPCODE1, LOW);

	// CPU Flash enable pins
	pinMode(PIN_CPU_WR_EN_S0, OUTPUT);
	pinMode(PIN_CPU_WR_EN_S1, OUTPUT);
	digitalWrite(PIN_CPU_WR_EN_S0, LOW);
	digitalWrite(PIN_CPU_WR_EN_S1, LOW);

	// Flash buffer enable pins
	pinMode(PIN_BUF_OE_S0, OUTPUT);
	pinMode(PIN_BUF_OE_S1, OUTPUT);
	digitalWrite(PIN_BUF_OE_S0, HIGH);
	digitalWrite(PIN_BUF_OE_S1, HIGH);

	// Flash triggers
	pinMode(PIN_TRIG_S0, OUTPUT);
	pinMode(PIN_TRIG_S1, OUTPUT);
	digitalWrite(PIN_TRIG_S0, HIGH);
	digitalWrite(PIN_TRIG_S1, HIGH);

	// Flash - input pins
	pinMode(PIN_ACTIVE_S0, INPUT);
	pinMode(PIN_ACTIVE_S1, INPUT);
	pinMode(PIN_PROTECTED_S0, INPUT);
	pinMode(PIN_PROTECTED_S1, INPUT);

	// RAM pins
	pinMode(PIN_RAM_LD_ADDR_SR0, OUTPUT);
	pinMode(PIN_RAM_WR_SR0, OUTPUT);
	pinMode(PIN_RAM_RD_SR0, OUTPUT);
	digitalWrite(PIN_RAM_LD_ADDR_SR0, LOW);
	// Note: Although typically these pins would be negative logic (and thus
	// should be high in default/off state), with BHM-159,1 they were converted
	// to positive logic and thus must default low/off. Note: Flipped back to
	// negative logic with BHM-159,3.
	digitalWrite(PIN_RAM_WR_SR0, HIGH);
	digitalWrite(PIN_RAM_RD_SR0, HIGH);

	// System state pins
	pinMode(PIN_USB_PROG, INPUT);
	pinMode(PIN_SYSTEM_PROGRAM, INPUT);

    //Get first data from backend before differentiating into READ vs WRITE mode...
    bool waiting_for_first_packet = true;
    while(waiting_for_first_packet){

        //Otherwise, wait for data
        if (Serial.available() > 0){

            //Load all available data
            while (Serial.available() > 0){

                //Read value
                int recd_val = Serial.read();

                //Only save values that aren't newlines or carriage returns
                if (recd_val != '\n' && recd_val != '\r'){
                    buffer[load_idx++] = recd_val;
                }else{
                    return;
                }


                //Prevent overflow
                if (load_idx >= LOAD_BUFFER_SIZE){
                    load_idx = 0;
                }
            }

            //Check for connection test (from data uploader -> WRITE mode)
            if (load_idx == 1 && (buffer[0] == 'T' || buffer[0] == 'C')){
                Serial.println("Hello from Arduino");
                in_write_mode = true;
                waiting_for_first_packet = false;
                load_idx = 0;

				// Check if erase signal was sent
				if (buffer[0] == 'C'){
					erase_before_write = true;
				}

            }

            //Check for connection test (from data downloader -> READ mode)
            if (load_idx == 1 && buffer[0] == 'R'){
                Serial.println("Hello from Arduino");
                in_write_mode = false;
                waiting_for_first_packet = false;
                load_idx = 0;
            }
        }
    }


	//Enable flash memory
	digitalWrite(PIN_CPU_WR_EN_S0, LOW);
	digitalWrite(PIN_CPU_WR_EN_S0, HIGH);
	digitalWrite(PIN_CPU_WR_EN_S0, LOW);

	if (erase_before_write){
		chip_erase();
	}


}

void loop() {

    if (in_write_mode){
        writer_loop();
    }else{
        reader_loop();
    }



}

//*****************************************************************************
//************** FROM EEPROM_WRITER.INO ***************************************

/*
Main loop function, copied from EEPROM_WRITER
*/
void writer_loop() {

    //Write main buffers to chip if possible
    if (run_write_cycle){

        //For each point
        size_t last_num_failed = 0;
        size_t num_failed = 0;
        for (size_t i  = 0 ; i < write_cycle_end ; i ++){

            //Write data
            if (!write_byte(addr_buf[i], data_buf[i])){
                num_failed++;
            }
            // delay(3); //ms

            //Send a heartbeat every 100 points
            if (i%50 == 0){
                if (num_failed > last_num_failed){
					Serial.println(error);
                    // Serial.println("E");
                }else{
                    Serial.println("U");
                }
                last_num_failed = num_failed;
            }
        }

        Serial.println("Ready");
        run_write_cycle = false;

    }

    //Otherwise, wait for data
    if (Serial.available() > 0){

        //Load all available data
        while (Serial.available() > 0){

            //Read value
            int recd_val = Serial.read();

            //Only save values that aren't newlines or carriage returns
            if (recd_val != '\n' && recd_val != '\r'){
                buffer[load_idx++] = recd_val;
            }else{
                return;
            }


            //Prevent overflow
            if (load_idx >= LOAD_BUFFER_SIZE){
                load_idx = 0;
            }
        }

        //Check for connection test
        if (load_idx == 1 && (buffer[0] == 'T' || buffer[0] == 'C')){
            Serial.println("Hello from Arduino");
            load_idx = 0;

			if (buffer[0] == 'C'){
				chip_erase();
			}
        }

        //Check for complete packet
        if (load_idx > 0 && buffer[load_idx-1] == '*'){

            //Check for end-of-data symbol
            if (load_idx == 4 && buffer[0] == 'E' && buffer[1] == 'N' && buffer[2] == 'D' && buffer[3] == '*'){
                run_write_cycle = true;
                write_cycle_end = main_load_idx;
                return;
            }

            if (process_input()){
                Serial.println("G");
            }else{
                Serial.println("load_idx: " + String(load_idx) + " Error: >" + error + "<");
                // Serial.println("Failed: "+error);
            }

            load_idx = 0;

            //Send reply
            // if(num_colon == 1){ //Is valid
            //     Serial.println("G");
            // }else{
            //     String s = "";
            //     for (int i = 0 ; i < load_idx ; i++){
            //         s = s + String(buffer[i]) + "_";
            //     }
            //     Serial.println("B" + String(num_colon) + " '" + s + "'");
            // }
        }

    }




}

/*
Reads the data in the input buffer (using load_idx to know bounds) and populates
the address and data buffers.
*/
bool process_input(){

    //Calcualte number of colons
    int num_colon = 0;
    size_t last_idx;
    for (int i = 0 ; i < load_idx ; i++){
        if (buffer[i] == ':'){
            num_colon ++;
            last_idx = i;
        }
    }

    // Return false if wrong number of colons
    if (num_colon != 1){
        error = "Wrong number of colons (" + String(num_colon) + ")";
        return false;
    }

    // Read address
    String as = "";
    bool is_zero = true;
    for (size_t i = 0 ;i < last_idx ; i++){
        if (buffer[i] == '\r' || buffer[i] == '\n'){
            as = as + "^";
        }else{
            as = as + String(char(buffer[i]));
        }
        if (buffer[i] != '0') is_zero = false;
    }
    long address = round(as.toInt());
    if (address == 0 and !is_zero){
        error = "Failed to interpret address (" + as + ")";
        return false;
    }

    // Read data
    String ds = "";
    is_zero = true;
    for (size_t i = last_idx+1 ;i < load_idx-1 ; i++){
        if (buffer[i] == '\r' || buffer[i] == '\n'){
            ds = ds + "^";
        }else{
            ds = ds + String(char(buffer[i]));
        }
        if (buffer[i] != '0') is_zero = false;
    }
    int data = round(ds.toInt());
    if (data == 0 and !is_zero){
        error = "Failed to interpret data (" + ds + ")";
        return false;
    }

    //Load data into main buffers
    addr_buf[main_load_idx] = address;
    data_buf[main_load_idx++] = data;

    if (main_load_idx >= DATA_BUFFER_SIZE){
        run_write_cycle = true;
        write_cycle_end = DATA_BUFFER_SIZE;
        main_load_idx = 0;
    }

    return true;

}

/*
Writes a byte to EEPROM chip
*/
bool write_byte(long address, int data){

    //Compute address array
	error = "";
    long bit_x_val;
    long address_sub = address;
    int address_bits[16];
    for (int i = 15 ; i >= 0 ; i--){ //For each address

        bit_x_val = lround(pow(2, i));

        if (address_sub - bit_x_val >= 0){
            address_sub -= bit_x_val;

            address_bits[i] = 1;
			error = error + "1";
        }else{
            address_bits[i] = 0;
			error = error + "0";
        }

    }

    //Compute data array
    int data_bits[8];
    int data_sub = data;
    for (int i = 7 ; i >= 0 ; i--){

        if (data > 255){
            error = "data out of bounds";
            return false;
        }


        bit_x_val = round(pow(2, i));
        // bit_x_val = bins[i];
        if (data_sub - bit_x_val >= 0){
            data_sub -= bit_x_val;
            data_bits[i] = 1;
			error = error + "1";
        }else{
            data_bits[i] = 0;
			error = error + "0";
        }

    }

    //Set ADDR to bus
    for (int  i = 0 ; i < 16 ; i++){
        digitalWrite(bus_pins_out[i], address_bits[i]);
    }

	// Enable write to address bus
	digitalWrite(PIN_WRADDR_BUS, HIGH);
	digitalWrite(PIN_WRDATA_BUS, LOW);

    //Write MAR
    digitalWrite(PIN_LD_ADDR_S0, LOW);
    digitalWrite(PIN_LD_ADDR_S0, HIGH);
    delayMicroseconds(write_time_74hc574);
    digitalWrite(PIN_LD_ADDR_S0, LOW);

	// Disable write to address bus
	digitalWrite(PIN_WRADDR_BUS, LOW);
	digitalWrite(PIN_WRDATA_BUS, LOW);

    //Set Data to bus
    for (int  i = 0 ; i < 8 ; i++){
        digitalWrite(bus_pins_out[i], data_bits[i]);
    }
	// for (int i = 0  ; i  < 16 ; i++){
    //     digitalWrite(bus_pins_out[i], HIGH);
	// }

	// Enable write to data bus
	digitalWrite(PIN_WRADDR_BUS, LOW);
	digitalWrite(PIN_WRDATA_BUS, HIGH);

    //Write Data
    digitalWrite(PIN_LD_DATA_S0, LOW);
    digitalWrite(PIN_LD_DATA_S0, HIGH);
    delayMicroseconds(write_time_74hc574);
    digitalWrite(PIN_LD_DATA_S0, LOW);

	// Disable write to data bus
	digitalWrite(PIN_WRADDR_BUS, LOW);
	digitalWrite(PIN_WRDATA_BUS, LOW);

    //Set opcodes
    digitalWrite(PIN_OPCODE0, HIGH);
    digitalWrite(PIN_OPCODE1, LOW);

    //Trigger operation
    digitalWrite(PIN_TRIG_S0, HIGH);
    digitalWrite(PIN_TRIG_S0, LOW);
    delayMicroseconds(trigger_operation_time);
    digitalWrite(PIN_TRIG_S0, HIGH);

    //Wait 'x' seconds (So active pin is definitely in correct state)
    delayMicroseconds(write_time_delay);

	// Wait for FMC to finish its routine
	while (digitalRead(PIN_ACTIVE_S0) == HIGH){
		delayMicroseconds(1);
	}

    //Set opcodes to read, in case something happens...
    digitalWrite(PIN_OPCODE0, LOW);
    digitalWrite(PIN_OPCODE1, HIGH);

    return true;
}

//*****************************************************************************
//************** FROM EEPROM_READER.INO ***************************************

/*
Setup function, copied from EEPROM_READER
*/
/*
void reader_setup() {
    Serial.begin(115200);

    for (size_t i = 0 ; i < 15 ; i++){
        pinMode(addr_pin[i], OUTPUT);
    }

    for (size_t i = 0 ; i < 8 ; i++){
        pinMode(data_pin[i], INPUT);
    }

    pinMode(PIN_WE, OUTPUT);
    pinMode(PIN_CE, OUTPUT);
    pinMode(PIN_OE, OUTPUT);


    digitalWrite(PIN_WE, HIGH);
    digitalWrite(PIN_CE, LOW);

}*/

/*
Main loop function, copied from EEPROM_READER
*/
void reader_loop() {

    //Read chip data into main buffers
    if (run_read_cycle){

        //Load address buffers with next addresses to write
        for (size_t i = 0 ; i < DATA_BUFFER_SIZE ; i++){

            //If at requested or max length, quit adding addresses and diable future reads
            read_cycle_end = -1;
            if (i+next_read_idx >= req_read_len || i >= file_len_max){
                run_read_cycle = false;
                read_cycle_end = i;
                break;
            }

            addr_buf[i] = next_read_idx + i;

        }
        next_read_idx = addr_buf[DATA_BUFFER_SIZE-1]+1;
        if (read_cycle_end == -1){
            read_cycle_end = DATA_BUFFER_SIZE;
        }


        //For each point
        size_t last_num_failed = 0;
        size_t num_failed = 0;
        for (size_t i  = 0 ; i < read_cycle_end ; i ++){

            //Read data
            if (!read_byte(addr_buf[i], data_buf[i])){
                num_failed++;
            }

            //Send a heartbeat every 100 points
            if (i%50 == 0){
                if (num_failed > last_num_failed){
                    Serial.println("E");
                }else{
                    Serial.println("U");
                }
                last_num_failed = num_failed;
            }
        }

        Serial.println("Ready");

        //Send read data back to computer - start downlink
        for (size_t i = 0 ; i < read_cycle_end ; i++){

            //Send data point
            Serial.println(String(addr_buf[i]) + ":" + String(data_buf[i]));

            //Wait for full serial packet to arrive
            delayMicroseconds(100);

            //Wait for serail data
            while(Serial.available() < 1){
                delayMicroseconds(1);
            }

            //Read seral data
            while(Serial.available() > 0){

                int recd_val = Serial.read();
                if (recd_val == 'G'){
                    break;
                }else if(recd_val == 'B'){
                    //Oh no an error occured!
                }

            }

        }

        if (run_read_cycle){
            Serial.println("E");
        }else{
            Serial.println("D");
        }

    }

    //Otherwise, wait for data
    if (Serial.available() > 0){

        //Load all available data
        while (Serial.available() > 0){

            //Read value
            int recd_val = Serial.read();

            //Only save values that aren't newlines or carriage returns
            if (recd_val != '\n' && recd_val != '\r'){
                buffer[load_idx++] = recd_val;
            }else{
                return;
            }


            //Prevent overflow
            if (load_idx >= LOAD_BUFFER_SIZE){
                load_idx = 0;
            }
        }

        //Check for connection test
        if (load_idx == 1 && buffer[0] == 'T'){
            Serial.println("Hello from Arduino");
            load_idx = 0;
        }

        //Check for complete packet
        if (load_idx > 0 && buffer[load_idx-1] == '*'){

            // //Check for end-of-data symbol
            // if (load_idx == 4 && buffer[0] == 'E' && buffer[1] == 'N' && buffer[2] == 'D' && buffer[3] == '*'){
            //     run_read_cycle = true;
            //     read_cycle_end = load_idx;
            //     return;
            // }

            if (process_req_input()){
                strcpy(charbuf, "G");
                dtostrf(req_read_len, 1, 1, charbuftemp);
                strcat(charbuf, charbuftemp);
                Serial.println(charbuf);
                run_read_cycle = true;
            }else{
                Serial.println("load_idx: " + String(load_idx) + " Error: >" + error + "<");
                // Serial.println("Failed: "+error);
            }

            load_idx = 0;

            //Send reply
            // if(num_equal == 1){ //Is valid
            //     Serial.println("G");
            // }else{
            //     String s = "";
            //     for (int i = 0 ; i < load_idx ; i++){
            //         s = s + String(buffer[i]) + "_";
            //     }
            //     Serial.println("B" + String(num_equal) + " '" + s + "'");
            // }
        }

    }




}

/*
Reads the data in the input buffer and populates the req_read_len variable.
*/
bool process_req_input(){

    //Verify length
    if (load_idx < 3){
        error = "Message was too short";
        return false;
    }

    //Calcualte number of equal signs
    int num_equal = 0;
    size_t last_idx;
    for (int i = 0 ; i < load_idx ; i++){
        if (buffer[i] == '='){
            num_equal ++;
            last_idx = i;
        }
    }

    // Return false if wrong number of equal signs
    if (num_equal != 1){
        error = "Wrong number of colons (" + String(num_equal) + ")";
        return false;
    }

    // Verify correct start
    if (buffer[0] != 'L' or buffer[1] != '='){
        error = "Incorrect start of message (" + String(buffer[0]) + String(buffer[1]) + ")";
        return false;
    }

    // Read data
    String ds = "";
    bool is_zero = true;
    for (size_t i = 2 ;i < load_idx-1 ; i++){
        if (buffer[i] == '\r' || buffer[i] == '\n'){
            ds = ds + "^";
        }else{
            ds = ds + String(char(buffer[i]));
        }
        if (buffer[i] != '0') is_zero = false;
    }
    size_t data = round(ds.toInt());
    if (data == 0 and !is_zero){
        error = "Failed to interpret length string (" + ds + ")";
        return false;
    }

    req_read_len = data;

    return true;

}

/*
Reads a byte from EEPROM chip
*/
bool read_byte(long address, int& data){

    long max_addr = 65535;

    // digitalWrite(24, HIGH);

    //Check address in bounds
    if (address > max_addr){
        error = "address out of bounds";
        return false;
    }

    //Compute address array
	error = "";
    float address_sub = address;
    int address_bits[16];
    for (int i = 15 ; i >= 0 ; i--){ //For each address

        if (address_sub - pow(2, i) >= 0){
            address_sub -= pow(2, i);

            address_bits[i] = 1;
			error = error + "1";
        }else{
            address_bits[i] = 0;
			error = error + "0";
        }

    }

    //Set ADDR to bus
    for (int  i = 0 ; i < 16 ; i++){
        digitalWrite(bus_pins_out[i], address_bits[i]);
    }

	// Enable write to address bus
	digitalWrite(PIN_WRADDR_BUS, HIGH);
	digitalWrite(PIN_WRDATA_BUS, LOW);

	// Write MAR
    digitalWrite(PIN_LD_ADDR_S0, LOW);
    delayMicroseconds(trigger_operation_time);
    digitalWrite(PIN_LD_ADDR_S0, HIGH);
    delayMicroseconds(trigger_operation_time);
    digitalWrite(PIN_LD_ADDR_S0, LOW);

	// Disable write to address bus
	digitalWrite(PIN_WRADDR_BUS, LOW);
	digitalWrite(PIN_WRDATA_BUS, LOW);

    //Set opcodes
    digitalWrite(PIN_OPCODE0, LOW);
    digitalWrite(PIN_OPCODE1, HIGH);

    //Trigger operation
    digitalWrite(PIN_TRIG_S0, HIGH);
    delayMicroseconds(trigger_operation_time);
    digitalWrite(PIN_TRIG_S0, LOW);
    delayMicroseconds(trigger_operation_time);
    digitalWrite(PIN_TRIG_S0, HIGH);

    //Wait 'x' seconds
    delayMicroseconds(read_time_delay);
	while (digitalRead(PIN_ACTIVE_S0) == HIGH){
		delayMicroseconds(1);
	}
    delay(50);




    //Read data lines
	digitalWrite(PIN_BUF_OE_S0, LOW);
    float data_float = 0;
    for (int i = 7 ; i >= 0 ; i--){

        if (digitalRead(bus_pins_in[i]) == HIGH) data_float += (pow(2.0, float(i)));

    }
    data = round(data_float);
	digitalWrite(PIN_BUF_OE_S0, HIGH);

    //Clear pause
    // digitalWrite(PIN_CLEAR_PAUSE, HIGH);
    // digitalWrite(PIN_CLEAR_PAUSE, LOW);
    // delayMicroseconds(clear_pause_time);
    // digitalWrite(PIN_CLEAR_PAUSE, HIGH);
	//
    // //Disable chip
    // digitalWrite(PIN_CHIP_ENABLE, HIGH);

    return true;
}

void chip_erase(){

	//Set opcodes (to chip erase)
    digitalWrite(PIN_OPCODE0, LOW);
    digitalWrite(PIN_OPCODE1, LOW);

    //Trigger operation
    digitalWrite(PIN_TRIG_S0, HIGH);
    digitalWrite(PIN_TRIG_S0, LOW);
    delayMicroseconds(trigger_operation_time);
    digitalWrite(PIN_TRIG_S0, HIGH);

	// Wait for FMC to finish its routine
	while (digitalRead(PIN_ACTIVE_S0) == HIGH){
		delayMicroseconds(1);
	}

    //Set opcodes to read, in case something happens...
    digitalWrite(PIN_OPCODE0, LOW);
    digitalWrite(PIN_OPCODE1, HIGH);

}
