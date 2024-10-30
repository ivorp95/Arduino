#! /usr/bin/env python3

import serial

if __name__ == "__main__":
    ser = serial.Serial('/dev/ttyUSB0',38400,timeout=1)
    ser.reset_input_buffer()

    while True:
        if ser.in_waiting > 0:
            val=ser.readline().decode('utf-8').rstrip()
            if int(val) > 750:                                 #kast u integer type(myobyect)
                #print("PREKO 750")
                print(val)
            else:
                print("NEMA STRUJE")

~
