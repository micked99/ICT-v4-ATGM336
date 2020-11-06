void TXtiming() // Timing
{
  // run additional scripts here to generate data prior to TX if there is a large delay involved.
  if ((timeStatus() == timeSet) && (second() == 0) && (minute() >= 0))
  {
    setGPStime();
    
    if ((minute() % 10 == 0) && (second() <= 0)) // TX WSPR standard message
    {
      telemetry_set = true;
      GPS_VCC_off(); delay(30);
      rf_on();
      freq = WSPR_FREQ;
      setModeWSPR(); // set WSPR standard mode
      encode(); // begin radio transmission
      rf_off(); delay(2);
      GPS_VCC_on(); //digitalWrite(3, HIGH);
    }
    
    else if ((minute() % 10 == 2) && (second() <= 0) && (telemetry_set == true))    // TX WSPR telemetry message
    {
      loc_dbm_telem(); // Update WSPR telemetry locator and dbm. Get temperature, voltage, speed in knots, GPS status and sats number
      GPS_VCC_off(); delay(30); 
      rf_on();
      freq = WSPR_FREQ;
      setModeWSPR_telem(); // set WSPR telemetry mode
      encode(); // begin radio transmission
      rf_off(); delay(2);
      GPS_VCC_on(); //digitalWrite(3, HIGH);
    }
    
    else if ((minute() % 10 == 4) && (second() <= 0) && (telemetry_set == true))    // TX WSPR standard message
    {
      GPS_VCC_off(); delay(30);
      rf_on();
      freq = WSPR_FREQ;
      setModeWSPR(); // set WSPR standard mode
      encode(); // begin radio transmission
      rf_off(); delay(2);
      GPS_VCC_on(); //digitalWrite(3, HIGH);
    }
    
    else if ((minute() % 10 == 6) && (second() <= 0) && (telemetry_set == true))    // TX WSPR standard message
    {
      GPS_VCC_off(); delay(30);
      rf_on();
      freq = WSPR_FREQ;
      setModeWSPR(); // set WSPR standard mode
      encode(); // begin radio transmission
      rf_off(); delay(2);
      GPS_VCC_on(); //digitalWrite(3, HIGH);
    }
    
    /*else if ((minute() % 10 == 8) && (second() <= 2)) // Check location age/validity and software GPS reset if necessary
    {
     
    }*/
    
    }
    }
