/*
  MHZ19.cpp - MH-Z19 CO2 sensor library for ESP8266 or Arduino
  version 1.0
  
  License MIT

  https://github.com/crisap94/MHZ19/
*/

#include "MHZ19_sensor.h"
#include "Arduino.h"

#define WAIT_READ_TIMES 100
#define WAIT_READ_DELAY 10

// public

MHZ19_sensor::MHZ19_sensor()
{
}

MHZ19_sensor::MHZ19_sensor(int rx, int tx)
{
    begin(rx, tx);
}

MHZ19_sensor::MHZ19_sensor(int pwm)
{
    begin(pwm);
}

MHZ19_sensor::~MHZ19_sensor()
{
}

void MHZ19_sensor::begin(int rx, int tx)
{
    _rx_pin = rx;
    _tx_pin = tx;
}

void MHZ19_sensor::begin(int pwm)
{
    _pwm_pin = pwm;
}

void MHZ19_sensor::setAutoCalibration(boolean autocalib)
{
    writeCommand(autocalib ? autocalib_on : autocalib_off);
}

void MHZ19_sensor::calibrateZero()
{
    writeCommand(zerocalib);
}

void MHZ19_sensor::calibrateSpan(int ppm)
{
    if (ppm < 1000)
        return;

    uint8_t cmd[MHZ19_sensor::REQUEST_CNT];
    for (int i = 0; i < MHZ19_sensor::REQUEST_CNT; i++)
    {
        cmd[i] = spancalib[i];
    }
    cmd[3] = (uint8_t)(ppm / 256);
    cmd[4] = (uint8_t)(ppm % 256);
    writeCommand(cmd);
}

/* undocumented function */
int MHZ19_sensor::getStatus()
{
    measurement_t m = getMeasurement();
    return m.state;
}

/* undocumented function, seems not to work with MH-Z19B */
boolean MHZ19_sensor::isWarming()
{
    return (getStatus() <= 1);
}

//protected
void MHZ19_sensor::writeCommand(uint8_t cmd[])
{
    writeCommand(cmd, NULL);
}

void MHZ19_sensor::writeCommand(uint8_t cmd[], uint8_t *response)
{
    SoftwareSerial mhz19_serial(_rx_pin, _tx_pin);
    mhz19_serial.begin(9600);
    mhz19_serial.write(cmd, REQUEST_CNT);
    mhz19_serial.write(mhz19_checksum(cmd));
    mhz19_serial.flush();

    if (response != NULL)
    {
        int i = 0;
        while (mhz19_serial.available() <= 0)
        {
            if (++i > WAIT_READ_TIMES)
            {
                Serial.println("error: can't get MH-Z19 response.");
                return;
            }
            yield();
            delay(WAIT_READ_DELAY);
        }
        mhz19_serial.readBytes(response, MHZ19_sensor::RESPONSE_CNT);
    }
}

//private

measurement_t MHZ19_sensor::getMeasurement()
{
    uint8_t buf[MHZ19_sensor::RESPONSE_CNT];
    for (int i = 0; i < MHZ19_sensor::RESPONSE_CNT; i++)
    {
        buf[i] = 0x0;
    }

    writeCommand(getppm, buf);
    // parse
    measurement_t measurement = {};
    if (buf[0] == 0xff && buf[1] == 0x86 && mhz19_checksum(buf) == buf[MHZ19_sensor::RESPONSE_CNT - 1])
    {
        measurement.co2_ppm = buf[2] * 256 + buf[3];
        measurement.temperature = buf[4] - 40;
        measurement.state = buf[5];
    }
    else
    {
        measurement.co2_ppm = measurement.temperature = measurement.state = -1;
    }
    return measurement;
}

int MHZ19_sensor::getPpmPwm()
{
    unsigned long th, tl, ppm = 0;

    do
    {
        th = pulseIn(_pwm_pin, HIGH, 1004000) / 1000;
        tl = 1004 - th;
        switch (PWM_DATA_SELECT)
        {
        case MHZ19_PWM_DATA::CALC_2000_PPM:
            ppm = 2000 * (th - 2) / (th + tl - 4);
            break;
        case MHZ19_PWM_DATA::CALC_5000_PPM:
            ppm = 5000 * (th - 2) / (th + tl - 4);
            break;
        }
    } while (th == 0);

    return ppm;
}

void MHZ19_sensor::setPwmData(MHZ19_PWM_DATA type)
{
    PWM_DATA_SELECT = type;
}

uint8_t MHZ19_sensor::mhz19_checksum(uint8_t com[])
{
    uint8_t sum = 0x00;
    for (int i = 1; i < MHZ19_sensor::REQUEST_CNT; i++)
    {
        sum += com[i];
    }
    sum = 0xff - sum + 0x01;
    return sum;
}
