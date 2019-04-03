import serial
import time
import struct
# Start with a basic flask app webpage.
from flask_socketio import SocketIO, emit, send
from flask import Flask, render_template, url_for, copy_current_request_context
from random import random   
from time import sleep                  
from threading import Thread, Event
import RPi.GPIO as GPIO
import voltage

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

sensor = voltage.PZEM()

class RandomThread(Thread):
    def __init__(self):
        self.delay = 1
        super(RandomThread, self).__init__()
        #Initialize AC Voltage/Current Sensor
        
    def measureCurrent(self):
        """
        Generate a random number every 1 second and emit to a socketio instance (broadcast)
        Ideally to be run in a separate thread?
        """
        #infinite loop to check current
        print("Making ra    ndom numbers")
        while not thread_stop_event.isSet():
            """
            number = round(random()*10, 3)
            print(number)
            """
            print("Checking Readiness")
            sensor.isReady()
            print(sensor.isReady())
            
            # Check if there's a load current
            print("Reading Current Sensor")
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
            
            socketio.emit('newnumber', {'number': "9"}, namespace='/test')
            sleep(self.delay)

    def run(self):
        self.measureCurrent()



                        
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
        thread = RandomThread()
        thread.start()

@socketio.on('disconnect', namespace='/test')
def test_disconnect():
    print('Client disconnected')

# Handing data from the webpage from toggle buttons
@socketio.on('message')
def handleMessage(msg):
    print('Message: ' + msg)
    #send(msg, broadcast=True)


if __name__ == '__main__':
    socketio.run(app)    
