#ifdef TFT_BUSY
    pinMode(TFT_BUSY, INPUT);
    pinMode(TFT_ENABLE, OUTPUT);
    digitalWrite(TFT_ENABLE, HIGH);
#endif
#ifdef ITE_ENABLE
    pinMode(ITE_ENABLE, OUTPUT);
    digitalWrite(ITE_ENABLE, HIGH);
#endif
    digitalWrite(TFT_RST, LOW);  
    delay(20);                   
    digitalWrite(TFT_RST, HIGH); 
    delay(10);     