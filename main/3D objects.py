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

accXx = 0
accXy = 0
accXz = 0

t = 0.102 #time between readings
y = 0 #total time

scene = canvas(width = 1000, height = 1000, background = color.white, align = "left")


l = 5 #length of rocket element

rocketVector = vector(0,0,0)
previousVector = vector(0,0,0)
rocket = box(pos = rocketVector, height = 3, width = 3, axis = vector(3, 0, 0) , color = color.orange, make_trail=True)



xarr = arrow(pos=rocket.pos, axis=vector(accXx,0,0), shaftwidth = .75, headwidth = 1, color=color.red, round=True) 
yarr = arrow(pos=rocket.pos, axis=vector(0,accXy,0), shaftwidth = .75, headwidth = 1, color=color.green, round=True) 
zarr = arrow(pos=rocket.pos, axis=vector(0,0,accXz), shaftwidth = .75, headwidth = 1, color=color.blue, round=True) 

g1 = graph(width=800,height=200,title='X acceleration', fast=False, align = "right")
f1 = gcurve(color = color.red) # graphics curve x

g12 = graph(width=800, height=200, title='X displacement', fast=False, align = "right")
f12 = gcurve(color = color.yellow) 

g2 = graph(width=800,height=200,title='Y acceleration', fast=False, align = "right")
f2 = gcurve(color = color.green) #graphics curve y

g22 = graph(width=800, height=200, title='Y displacement', fast=False, align = "right")
f22 = gcurve(color = color.orange)

g3 = graph(width=800, height=200, title='Z Acceleration', fast=False, align = "right")
f3 = gcurve(color = color.purple)

g32 = graph(width=800, height=200, title='Z displacement', fast=False, align = "right")
f32 = gcurve(color = color.black)

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

    accXy = accY * 100 #(float(accX)*sin(math.radians(float(pitch)))) #cm/s^2
    accXx = accX  * 100 #(float(accX)*cos(math.radians(float(pitch)))*cos(math.radians(float(yaw)))) #cm/s^2
    accXz = accZ * 100  #(float(accX)*cos(math.radians(float(pitch)))*sin(math.radians(float(yaw)))) #cm/s^2


for x in f:
    timerun = time.time()
    filteredLine(x)
    # rocketVector = vector(int(accX), int(accY), int(accZ))  


    sx = (ux*t)+(0.5*accXx)*t**2
    sy = (uy*t)+(0.5*accXy)*t**2
    sz = (uz*t)+(0.5*accXz)*t**2

    ux = ux + accXx*t
    uy = uy + accXy*t
    uz = uz + accXz*t

    f1.plot(y, accX)
    f12.plot(y, sx)
    f2.plot(y, accY)
    f22.plot(y, sy)
    f3.plot(y, accZ)
    f32.plot(y, sz)

    xarr.pos = vector(sx, sy, sz)
    yarr.pos = vector(sx, sy, sz)
    zarr.pos = vector(sx, sy, sz)

    # if int(timerun) % 2 == 0:
    #     rocket.axis = rocket.pos

    rocket.pos = vector(sx, sy, sz)

    
    
    

    xarr.axis = vector(accXx*t,0,0)
    yarr.axis = vector(0,accXy*t,0)
    zarr.axis = vector(0,0,accXz*t)
    
    y = y + t
    

    # rocketVector.x = sx
    # rocketVector.y = sy
    # rocketVector.z = sz

    # rocket.pos = rocketVector
    # arrow1.pos = rocketVector
    scene.camera.follow(rocket)


    print(sx, sy, sz, pitch, roll, yaw)
    time.sleep(t)
    


