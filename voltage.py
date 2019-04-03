#!/usr/bin/env python3 
#coding=utf-8

# Code by Massi from  https://www.raspberrypi.org/forums/viewtopic.php?t=124958#p923274


# PDAControl
# Documentation PDAControl English:
# http://pdacontrolen.com/meter-pzem-004t-with-arduino-esp32-esp8266-python-raspberry-pi/
# Documentacion PDAControl Español:
# http://pdacontroles.com/medidor-pzem-004t-con-arduino-esp32-esp8266-python-raspberry-pi/
# Video Tutorial : https://youtu.be/qt32YT_1oH8


import serial
import time
import struct
import RPi.GPIO as GPIO
# Start with a basic flask app webpage.
from flask_socketio import SocketIO, emit
from flask import Flask, render_template, url_for, copy_current_request_context
from random import random   
from time import sleep                  
from threading import Thread, Event

__author__ = 'BenBequette'

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
app.config['DEBUG'] = True

#turn the flask app into a socketio app
socketio = SocketIO(app)

#Initialize GPIO
RELAY_START = 20 # GPIO pin -> to Arduino A1 Pin
RELAY_END = 21 # GPIO pin -> to Arduino A2 pin
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.setup(RELAY_START, GPIO.OUT)
GPIO.setup(RELAY_END, GPIO.OUT)
GPIO.output(RELAY_START, False) 
GPIO.output(RELAY_END, False)

#Set Upper & Lower Limits for the Current
AMP_UPPER_LIMIT = 1.0 #~A: Turn ON when greater than
AMP_LOWER_LIMIT = 1.0 #~A: Shut OFF when lower than


#random number Generator Thread
thread = Thread()
thread_stop_event = Event()
class PZEM():

    setAddrBytes        =   [0xB4,0xC0,0xA8,0x01,0x01,0x00,0x1E]
    readVoltageBytes    =   [0xB0,0xC0,0xA8,0x01,0x01,0x00,0x1A]
    readCurrentBytes    =   [0XB1,0xC0,0xA8,0x01,0x01,0x00,0x1B]
    readPowerBytes      =   [0XB2,0xC0,0xA8,0x01,0x01,0x00,0x1C]
    readRegPowerBytes   =   [0XB3,0xC0,0xA8,0x01,0x01,0x00,0x1D]

    # dmesg | grep tty type this in terminal to list Serial linux command
    
    def __init__(self, com="/dev/ttyUSB0", timeout=10.0):       # Usb serial port          
    #def __init__(self, com="/dev/ttyUSB1", timeout=10.0):       # Usb serial port               
    #def __init__(self, com="/dev/ttyAMA0", timeout=10.0):       # Raspberry Pi port Serial TTL         
    #def __init__(self,com="/dev/rfcomm0", timeout=10.0):
    
        self.ser = serial.Serial(
            port=com,
            baudrate=9600,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout = timeout
        )
        if self.ser.isOpen():
            self.ser.close()
            self.ser.open()

    def checkChecksum(self, _tuple):
        _list = list(_tuple)
        _checksum = _list[-1]
        _list.pop()
        _sum = sum(_list)
        if _checksum == _sum%256:
            return True
        else:
            raise Exception("Wrong checksum")
            
    def isReady(self):
        self.ser.write(serial.to_bytes(self.setAddrBytes))
        rcv = self.ser.read(7)
        if len(rcv) == 7:
            unpacked = struct.unpack("!7B", rcv)
            if(self.checkChecksum(unpacked)):
                return True
        else:
            raise serial.SerialTimeoutException("Timeout setting address")

    def readVoltage(self):
        self.ser.write(serial.to_bytes(self.readVoltageBytes))
        rcv = self.ser.read(7)
        if len(rcv) == 7:
            unpacked = struct.unpack("!7B", rcv)
            if(self.checkChecksum(unpacked)):
                tension = unpacked[2]+unpacked[3]/10.0
                return tension
        else:
            raise serial.SerialTimeoutException("Timeout reading tension")

    def readCurrent(self):
        self.ser.write(serial.to_bytes(self.readCurrentBytes))
        rcv = self.ser.read(7)
        if len(rcv) == 7:
            unpacked = struct.unpack("!7B", rcv)
            if(self.checkChecksum(unpacked)):
                current = unpacked[2]+unpacked[3]/100.0
                return current
        else:
            raise serial.SerialTimeoutException("Timeout reading current")

    def readPower(self):
        self.ser.write(serial.to_bytes(self.readPowerBytes))
        rcv = self.ser.read(7)
        if len(rcv) == 7:
            unpacked = struct.unpack("!7B", rcv)
            if(self.checkChecksum(unpacked)):
                power = unpacked[1]*256+unpacked[2]
                return power
        else:
            raise serial.SerialTimeoutException("Timeout reading power")

    def readRegPower(self):
        self.ser.write(serial.to_bytes(self.readRegPowerBytes))
        rcv = self.ser.read(7)
        if len(rcv) == 7:
            unpacked = struct.unpack("!7B", rcv)
            if(self.checkChecksum(unpacked)):
                regPower = unpacked[1]*256*256+unpacked[2]*256+unpacked[3]
                return regPower
        else:
            raise serial.SerialTimeoutException("Timeout reading registered power")

    def readAll(self):
        if(self.isReady()):
            return(self.readVoltage(),self.readCurrent(),self.readPower(),self.readRegPower())


    def run(self):
        self.readAll()

    def close(self):
        self.ser.close()



@app.route('/')
def index():
    #only by sending this page first will the client be connected to the socketio instance
    return render_template('index.html')

@socketio.on('connect', namespace='/test')
def test_connect():
    # need visibility of the global thread object
    global thread
    print('Client connected')

    #Start the random number generator thread only if the thread has not been started before.
    if not thread.isAlive():
        print("Starting Thread")
        thread = PZEM()
        thread.start()

@socketio.on('disconnect', namespace='/test')
def test_disconnect():
    print('Client disconnected')

def test_all_readings():
    while 1:
        print("Checking readiness")
        print(sensor.isReady())
        print("Reading voltage")
        print(sensor.readVoltage())
        print("Reading current")
        print(sensor.readCurrent())
        print("Reading power")
        print(sensor.readPower())
        print("reading registered power")
        print(sensor.readRegPower())
        print("reading all")
        print(sensor.readAll())

# Main Routine
if __name__ == '__main__':
    #test_all_readings() #Uncomment if you want to test
    #Run Webserver
    socketio.run(app)
    
    #Initialize AC Voltage/Current Sensor
    sensor = PZEM()

    # Main Loop
    while 1:
        print("Checking Readiness")
        sensor.isReady()
        print(sensor.isReady())
        # Check if there's a load current
        print("Reading Sensor")
        print(sensor.readCurrent())
        if sensor.readCurrent() > AMP_UPPER_LIMIT:
            # Send GPIO Signal to Arduino to Switch Load Relay
            print("Current Detected. Starting Sequence")
            GPIO.output(RELAY_START, True)
            GPIO.output(RELAY_END, False)
        elif sensor.readCurrent() < AMP_LOWER_LIMIT:
            #Send GPIO Signal to Arduino to Switch Off
            print("Low Current Detected. Stopping Sequence")
            GPIO.output(RELAY_START, False)
            GPIO.output(RELAY_END, True)
        
        socketio.emit('newnumber', {'number': sensor.readVoltage()}, namespace='/test')
 
