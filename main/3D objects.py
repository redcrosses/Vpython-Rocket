from vpython import *

from pathlib import Path

import math

import time

data = Path(r'main/data.txt')

f = open(data, "r")

previousx = 0
previousy = 0
previousz = 0

sx = 0 #displacement x
sy = 0 #displacement y
sz = 0 #displacement z

vx = 0 #final velocity x
vy = 0 #final velocity y
vz = 0 #final velocity z

ux = 0 #initial velocity x
uy = 0 #initial velocity y
uz = 0 #initial velocity z

vx = 0 #final velocity

t = 0.102 #time between readings


sphereVector = vector(0,0,0)

sphere1 = sphere(pos = sphereVector, radius = 0.5, color = color.orange, make_trail=True)

varr = arrow(pos=sphere1.pos, axis=vector(sx+3,sy+3,sz+3), color=color.yellow, round=True) 

# sample of data: "  -1184   3120 -15824"

def filteredLine(line):
    global accX
    global accY
    global accZ
    global pitch
    global roll
    global yaw
    global previous
    global previousx 
    global previousy 
    global previousz 
    global accXx
    global accXy
    global accXz
    
    accX = ""
    accY = ""
    accZ = ""

    accXx = 0
    accXy = 0
    accXz = 0

    pitch = ""
    roll = ""
    yaw = ""

    previous = " "
    counter = 0

    for char in line:
        if previous != " " and char == " ":
            previous = " "
            counter = counter + 1
            continue
        else:
            previous = char
        
        if(char != " " and counter == 0): #assigns the X in the model
            accX = accX + char
        
        if(char != " " and counter == 1): #assigns the Y in the model
            accY = accY + char

        if(char != " " and counter == 2): # assigns the Z in the model
            accZ = accZ + char

        if(char != " " and counter == 3): # assigns the pitch
            pitch = pitch + char
        
        if(char != " " and counter == 4): # assigns the roll
            roll = roll + char
        
        if(char != " " and counter == 5): # assigns the yaw
            yaw = yaw + char
            
        # conversion factor of 16, because the lowest 4 bits of the accelerometer's readings are 0.


    accX = (float(accX) / -16 / 1000 * 9.81) - previousx #m/s^2
    accY = (float(accY) / -16 / 1000 * 9.81) - previousy #m/s^2

    accZ = (float(accZ) / -16 / 1000 * 9.81) - previousz #m/s^2

    previousx = accX
    previousy = accY
    previousz = accZ

    accXy = (float(accX)*sin(math.radians(float(pitch))))
    accXx = (float(accX)*cos(math.radians(float(pitch)))*cos(math.radians(float(yaw))))
    accXz = (float(accX)*cos(math.radians(float(pitch)))*sin(math.radians(float(yaw))))

for x in f:
    filteredLine(x)
    # sphereVector = vector(int(accX), int(accY), int(accZ))  


    sx = (ux*t)+(0.5*accXx)*t**2
    sy = (uy*t)+(0.5*accXy)*t**2
    sz = (uz*t)+(0.5*accXz)*t**2

    ux = ux + accXx
    uy = uy + accXy
    uz = uz + accXz


    sphere1.pos = vector(sx, sy, sz)
    varr.pos = vector(sx, sy, sz)
    varr.axis = vector(ux,uy,uz)

    # sphereVector.x = sx
    # sphereVector.y = sy
    # sphereVector.z = sz

    # sphere1.pos = sphereVector
    # arrow1.pos = sphereVector
    scene.center = sphere1.pos

    print(sx, sy, sz, pitch, roll, yaw)
    time.sleep(0.05)
    


