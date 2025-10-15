#ifdef TFT_BUSY
    pinMode(TFT_BUSY, INPUT);
    pinMode(TFT_ENABLE, OUTPUT); 
    pinMode(TFT_CS1, OUTPUT); 
    digitalWrite(TFT_ENABLE, HIGH);  
    digitalWrite(TFT_CS1, HIGH); 
#endif
