#include <Arduino.h>

#include <SPI.h>
#include <RP2040_SD.h>

#define PIN_SD_MOSI PIN_SPI0_MOSI
#define PIN_SD_MISO PIN_SPI0_MISO
#define PIN_SD_SCK PIN_SPI0_SCK
#define PIN_SD_SS PIN_SPI0_SS





void setupStorage()
{
    if (!SD.begin(PIN_SD_SS))
    {
        Serial.println("Initialization failed!");
        return;
    }

    Serial.println("Initialization done.");
}



void saveFile(String dataString)
{

    File dataFile = SD.open("datalog.csv", FILE_WRITE);

    if (dataFile && dataString != 0)
    {
        dataFile.println(dataString);

        // print to the serial port too:
        Serial.println(dataString);
    }
    else if (dataString == 0)
    {
    }
    // if the file isn't open, pop up an error:
    else
    {
        Serial.println("Error opening: datalog.txt ");
    }

    dataFile.close();
}


