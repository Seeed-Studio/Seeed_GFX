/*
 * GDEB0709E01 - pin initialisation for the BOE 7.09" six-color ePaper
 * panel. Identical to T133A01: the dual-COG board needs TFT_CS1, power is
 * gated by TFT_ENABLE, and a reset pulse precedes the command sequence.
 */
#ifdef TFT_BUSY
    pinMode(TFT_BUSY, INPUT);
#endif
#ifdef TFT_ENABLE
    pinMode(TFT_ENABLE, OUTPUT);
    digitalWrite(TFT_ENABLE, HIGH);
#endif
    pinMode(TFT_CS1, OUTPUT);
    digitalWrite(TFT_CS1, HIGH);
    digitalWrite(TFT_RST, LOW);
    delay(20);
    digitalWrite(TFT_RST, HIGH);
    delay(20);
