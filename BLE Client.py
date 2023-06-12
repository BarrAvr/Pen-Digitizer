# https://medium.com/@protobioengineering/how-to-connect-to-a-bluetooth-device-with-a-macbook-and-python-7a14ece6a780

import asyncio
from bleak import BleakScanner, BleakClient, BleakGATTCharacteristic
import time
import turtle
import win32api, win32con

CURSOR_MODE = False
TURTLE_MODE = True

def reset_turtle():
    turtle.clear()
    t_x = 0
    t_y = 0
    turtle.penup()
    turtle.setpos(0, 0)

def callback(sender: BleakGATTCharacteristic, data: bytearray):
    # print(f"Notification: {sender}: {data}")
    decoded = data.decode('utf-8')
    xyz = decoded.split(',')
    # print(xyz)

    x,y,z,mode = float(xyz[0]), float(xyz[1]), -float(xyz[2]), int(xyz[3])
    if mode == 0:
        if TURTLE_MODE:
            turtle.penup()
    elif mode == 1:
        if TURTLE_MODE:
            turtle.pendown()
        
        if CURSOR_MODE:
            c_x,c_y = win32api.GetCursorPos()
            win32api.mouse_event(win32con.MOUSEEVENTF_RIGHTDOWN,c_x,c_y,0,0)
            win32api.mouse_event(win32con.MOUSEEVENTF_RIGHTUP,c_x,c_y,0,0)
    elif mode == 2:
        if TURTLE_MODE:
            reset_turtle()
        
        if CURSOR_MODE:
            c_x,c_y = win32api.GetCursorPos()
            # win32api.SetCursorPos((0,0))
            win32api.mouse_event(win32con.MOUSEEVENTF_LEFTDOWN,c_x,c_y,0,0)
            win32api.mouse_event(win32con.MOUSEEVENTF_LEFTUP,c_x,c_y,0,0)
    if TURTLE_MODE:
        t_x,t_y = turtle.pos()
        turtle.setpos(t_x + z, t_y + y)
        # if off-screen, reset
        t_x,t_y = turtle.pos()
        ss_x, ss_y = turtle.window_width(), turtle.window_height()
        if t_x < -ss_x/2 or t_x > ss_x/2 or t_y < -ss_y/2 or t_y > ss_y/2:
            pass
            # reset_turtle()
        # print(int.from_bytes(data,byteorder='little'))
    if CURSOR_MODE:
        c_x,c_y = win32api.GetCursorPos()
        win32api.SetCursorPos((c_x + int(z), c_y - int(y)))

async def main():
    devices = await BleakScanner.discover()
    for device in devices:
        print(device)
    
    for device in devices:
        if device.name == "CS147":
            async with BleakClient(device.address, winrt=dict(use_cached_services=False)) as client:
                if client.is_connected:
                    # svcs = await client.get_services()
                    if TURTLE_MODE:
                        turtle.screensize()
                        turtle.color('red')
                        turtle.left(120)
                        # turtle.hideturtle()

                    await client.start_notify("beb5483e-36e1-4688-b7f5-ea07361b26a8", callback)
                    await asyncio.sleep(10000000)
                else:
                    print("Client not connected.")



asyncio.run(main())