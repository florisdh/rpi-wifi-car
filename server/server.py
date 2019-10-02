from Adafruit_MotorHAT import Adafruit_MotorHAT, Adafruit_DCMotor
import socket
import struct
import atexit
import math

# Default Values
recBufferSize = 512
port = 1337
angleMotorIndex = 4
speedMotorIndex = 1
angle = 0
speed = 0

# Load MotorHAT
motorHAT = Adafruit_MotorHAT(addr=0x60)

# Stop motors on app close
def StopMotors():
    angleMotor.run(Adafruit_MotorHAT.RELEASE)
    speedMotor.run(Adafruit_MotorHAT.RELEASE)
    speed = 0
    angle = 0
atexit.register(StopMotors)

# Start Server
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
server.bind(("", port))
server.listen(1)

print("Loaded Server")

# Main App loop
recBuffer = bytearray(recBufferSize)
recView = memoryview(recBuffer)
while True:
    # Accept connection request
    (con, ip) = server.accept()
    print("Connection: " + str(ip))

    # Load motors
    angleMotor = motorHAT.getMotor(angleMotorIndex)
    speedMotor = motorHAT.getMotor(speedMotorIndex)

    # Client loop
    while True:
        # Receive data
        try:
            amount = con.recv_into(recBuffer, recBufferSize)
        except:
            print("Connection lost to " + str(ip))
            break
        
        # Check all messages
        i = 0
        while (i + 2 < amount):
            # Format data
            type = int(recBuffer[i])
            val = int(struct.unpack_from("h", buffer(recBuffer), i + 1)[0])

            # Compile
            if type == 1: # Change Angle
                if angle != val:
                    # Select direction
                    if angle >= 0 and val < 0: # Rotate to left
                        angleMotor.run(Adafruit_MotorHAT.BACKWARD)
                    elif angle <= 0 and val > 0: # Rotate to right
                        angleMotor.run(Adafruit_MotorHAT.FORWARD)
                
                    # Apply angle
                    angle = val
                    angleMotor.setSpeed(int(math.fabs(angle)))

                    #print("Changing angle to: " + str(angle))

            elif type == 2: # Change Speed
                if speed != val:
                    # Select direction
                    if speed >= 0 and val < 0: # Rotate to left
                        speedMotor.run(Adafruit_MotorHAT.BACKWARD)
                    elif speed <= 0 and val > 0: # Rotate to right
                        speedMotor.run(Adafruit_MotorHAT.FORWARD)

                    # Apply speed
                    speed = val
                    speedMotor.setSpeed(int(math.fabs(speed)))

                    #print("Change speed to: " + str(speed))

            else:
                print("Invalid CMD: " + str(type) + " and VALUE " + str(val))

            # Go to next msg
            i += 3
    
    # Reset motors before next client
    StopMotors()
