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

// long addr_pin[15] = {46, 44, 42, 40, 38, 36, 34, 32, 33, 35, 41, 37, 30, 31, 28}; //From 0-14
// int data_pin[8] = {48, 50, 52, 53, 51, 49, 47, 45}; //From 0-7

int data_rd_pin[8] = {2, 3, 4, 5, 6, 7, 8, 9}; //From 0 to 7
long addr_pin[15] = {22, 24, 26, 28, 30, 32, 34, 36,   40, 42, 44, 46, 48, 50, 52}; //From 0-14
int data_wr_pin[8] = {23, 25, 27, 29, 31, 33, 35, 37};
int ctrl_pin[4] = {53, 51, 49, 47}; //From 1 to 5 (pin 0 is ground)

#define PIN_STATUS_READ 14
#define PIN_STATUS_WRITE 15
#define PIN_STATUS_LINK 16
#define PIN_STATUS_ERROR 18
#define PIN_STATUS_PROGRAM 17


#define PIN_WE ctrl_pin[2]
// #define PIN_CE
#define PIN_OE ctrl_pin[1]
#define PIN_PRGM ctrl_pin[0]
#define PIN_WR_DATA ctrl_pin[3]

bool run_write_cycle = false;
size_t write_cycle_end = DATA_BUFFER_SIZE;

char charbuf[120];
char charbuftemp[20];

size_t req_read_len = 0;
bool run_read_cycle = false;
size_t next_read_idx = 0;
size_t read_cycle_end = DATA_BUFFER_SIZE;

bool in_write_mode = false;

void setup(){

    Serial.begin(115200);

	pinMode(PIN_STATUS_READ, OUTPUT);
	pinMode(PIN_STATUS_WRITE, OUTPUT);
	pinMode(PIN_STATUS_LINK, OUTPUT);
	pinMode(PIN_STATUS_ERROR, OUTPUT);
	pinMode(PIN_STATUS_PROGRAM, OUTPUT);

	digitalWrite(PIN_STATUS_READ, LOW);
	digitalWrite(PIN_STATUS_WRITE, LOW);
	digitalWrite(PIN_STATUS_LINK, LOW);
	digitalWrite(PIN_STATUS_PROGRAM, LOW);
	digitalWrite(PIN_STATUS_ERROR, LOW);


	pinMode(PIN_WE, OUTPUT);
	pinMode(PIN_WR_DATA, OUTPUT);
	pinMode(PIN_OE, OUTPUT);
	pinMode(PIN_PRGM, INPUT);

	for (size_t i = 0 ; i < 15 ; i++){
		pinMode(addr_pin[i], OUTPUT);
	}

	for (size_t i = 0 ; i < 8 ; i++){
		pinMode(data_wr_pin[i], OUTPUT);
	}

	for (size_t i = 0 ; i < 8 ; i++){
		pinMode(data_rd_pin[i], INPUT);
	}

	digitalWrite(PIN_OE, HIGH);
	digitalWrite(PIN_WE, HIGH);
	digitalWrite(PIN_WR_DATA, HIGH);

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
            if (load_idx == 1 && buffer[0] == 'T'){
                Serial.println("Hello from Arduino");
                in_write_mode = true;
                waiting_for_first_packet = false;
                load_idx = 0;

            }

            //Check for connection test (from data downloader -> READ mode)
            if (load_idx == 1 && buffer[0] == 'R'){
                Serial.println("Hello from Arduino");
                in_write_mode = false;
                waiting_for_first_packet = false;
                load_idx = 0;
            }

            // //Check for complete packet
            // if (load_idx > 0 && buffer[load_idx-1] == '*'){
            //
            //     if (process_req_input()){
            //         strcpy(charbuf, "G");
            //         dtostrf(req_read_len, 1, 1, charbuftemp);
            //         strcat(charbuf, charbuftemp);
            //         Serial.println(charbuf);
            //         run_read_cycle = true;
            //     }else{
            //         Serial.println("load_idx: " + String(load_idx) + " Error: >" + error + "<");
            //     }
            //
            //     load_idx = 0;
            // }

        }


    }




}

void loop() {

	digitalWrite(PIN_STATUS_LINK, LOW);

	// Set program light
	if (digitalRead(PIN_PRGM) == LOW){
		digitalWrite(PIN_STATUS_PROGRAM, LOW);
	}else{
		digitalWrite(PIN_STATUS_PROGRAM, HIGH);
	}

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

		digitalWrite(PIN_STATUS_WRITE, HIGH);

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

		digitalWrite(PIN_STATUS_WRITE, LOW);

    }

    //Otherwise, wait for data
    if (Serial.available() > 0){



        //Load all available data
        while (Serial.available() > 0){

			digitalWrite(PIN_STATUS_LINK, HIGH);

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

		digitalWrite(PIN_STATUS_LINK, LOW);

        //Check for connection test
        if (load_idx == 1 && buffer[0] == 'T'){
            Serial.println("Hello from Arduino");
            load_idx = 0;
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

		digitalWrite(PIN_STATUS_LINK, LOW);
    }else{
		digitalWrite(PIN_STATUS_LINK, LOW);
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

    //Disable outputs
    digitalWrite(PIN_OE, HIGH);

    //Disable write
    digitalWrite(PIN_WE, HIGH);

	//Set data read/write mode to write
	digitalWrite(PIN_WR_DATA, LOW);

    long max_addr = 32767;

    //Check address in bounds
    if (address > max_addr){
        error = "address out of bounds";
        return false;
    }

    //Set address lines
	error = "";
    float address_sub = address;
    for (int i = 15 ; i >= 0 ; i--){ //For each address

        if (address_sub - pow(2, i) >= 0){
            address_sub -= pow(2, i);
            digitalWrite(addr_pin[i], HIGH);
			// if (address == 0){
				error = error + "1";
			// }
        }else{
            digitalWrite(addr_pin[i], LOW);
			// if (address == 0){
				error = error + "0";
			// }
        }

    }

	// if (address == 0){
		error = error + " ";
	// }

    //Set data lines
    float data_sub = data;
    for (int i = 7 ; i >= 0 ; i--){

        if (data > 255){
            error = "data out of bounds";
            return false;
        }

        if (data_sub - pow(2, i) >= 0){
            data_sub -= pow(2, i);
            digitalWrite(data_wr_pin[i], HIGH);
			// if (address == 0){
				error = error + "1";
			// }
        }else{
            digitalWrite(data_wr_pin[i], LOW);
			// if (address == 0){
				error = error + "0";
			// }
        }

    }

	if (address == 0){
		error = error + "<" + String(address);

		error = "WCE: " + String(write_cycle_end);
	}

    //Set chip select
    // digitalWrite(PIN_CE, LOW);
	digitalWrite(PIN_OE, HIGH);

    //Set write pin
	digitalWrite(PIN_WE, HIGH);
    digitalWrite(PIN_WE, LOW);
    delayMicroseconds(1);

    //Raise write pin
    digitalWrite(PIN_WE, HIGH);

	//Set data read/write mode back to read
	digitalWrite(PIN_WR_DATA, HIGH);

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

		digitalWrite(PIN_STATUS_READ, HIGH);

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

		digitalWrite(PIN_STATUS_READ, LOW);
		digitalWrite(PIN_STATUS_LINK, HIGH);

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

		digitalWrite(PIN_STATUS_LINK, LOW);

        if (run_read_cycle){
            Serial.println("E");
        }else{
            Serial.println("D");
        }



    }

    //Otherwise, wait for data
    if (Serial.available() > 0){ //TODO: Should this be deleted?

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

    //Calculate number of equal signs
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

    //Disable outputs
    digitalWrite(PIN_OE, HIGH);

    //Disable write
    digitalWrite(PIN_WE, HIGH);

	//Set data read/write mode to read
	digitalWrite(PIN_WR_DATA, HIGH);

    long max_addr = 32767;

    //Check address in bounds
    if (address > max_addr){
        error = "address out of bounds";
        return false;
    }

    //Set address lines
    float address_sub = address;
    for (int i = 14 ; i >= 0 ; i--){ //For each address

        if (address_sub - pow(2, i) >= 0){
            address_sub -= pow(2, i);
            digitalWrite(addr_pin[i], HIGH);
        }else{
            digitalWrite(addr_pin[i], LOW);
        }

    }

    //Set chip select
    // digitalWrite(PIN_CE, LOW);
    digitalWrite(PIN_OE, LOW);
    delayMicroseconds(1);

    //Read data lines
    float data_float = 0;

    strcpy(charbuf, "");
    //    Serial.print("\n\t");
    for (int i = 7 ; i >= 0 ; i--){



        if (digitalRead(data_rd_pin[i]) == HIGH){

            // dtostrf(pow(2.0, float(i)), 3, 1, charbuftemp);
            // strcat(charbuf, charbuftemp);
            // strcat(charbuf, "+");
            data_float += (pow(2.0, float(i)));
        }


    }

    // strcat(charbuf, "=");
    // dtostrf(data_float, 3, 1, charbuftemp);
    // strcat(charbuf, charbuftemp);
    // Serial.println(charbuf);
    data = round(data_float);

    // digitalWrite(PIN_OE, HIGH);

    return true;

}
