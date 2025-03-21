# Install ArduinoIDE
- Go to the [ArduinoIDE](https://www.arduino.cc/en/software) page
    - Click **Windows**
    - Click **Just Download**
    - Again, You don't need to enter or tick any information. Click **Just Download**
    - ***Done***

# Set Libraries on ArduinoIDE
- <h3>For STM32F103C8T6 Microprocessor</h3>
  - Launch Arduino IDE. Click on the "File" menu and then "Preferences".\
      The "Preferences" dialog will open, then add the following link to the "Additional Boards Managers URLs" field:
   "(https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json)". 
    
    Click "Ok"
  - Click on the "Tools" menu and then "Boards > Boards Manager." 
    - _The board manager will open, and you will see a list of installed and available boards._ 
    - **Select the "STM32 F103Cxxx" and click on Install.** 
    - _After installation is complete, an "INSTALLED" tag will appear next to the core name._ 
    - _You can close the Board Manager._ 
    - _You can find the STM32 boards package in the "Board" menu._ 
    - **Select the desired board series: STM32F103Cxxx** 
    - **Select the board.** 
  > This instructions are taken from [**Autodesk Instructables**](https://www.instructables.com/Getting-Started-With-Stm32-Using-Arduino-IDE/)

  > Follow this Link for visual instructions [**ST Community**](https://community.st.com/t5/stm32-mcus/how-to-program-and-debug-the-stm32-using-the-arduino-ide/ta-p/608514)


- <h3>For Sensors Libraries</h3>

  - ***For Hear Rate and SpO2 Sensor (MAX30102)***
      - SparkFun_MAX3010x_Pulse_and_Proximity_Sensor_Library
      - MAX30102
      - DFRobot MAX30102 Library

  - ***For ECG Sensor (AD8232)***
      - Adafruit AD8232 Library

  - ***For Display on OLED I2C***
      - Adafruit_GFX_Library
      - Adafruit_SSD1306
      - ssd1306

   > To Install Library in ArduinoIDE Go To Tools -> Manage Library -> 'Search By Name'
   
   
   Credit: Assaduzzaman Munna (https://github.com/iam-ajmunna/)

