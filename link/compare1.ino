// From FLASH_IO_V2.ino

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

    }




}
