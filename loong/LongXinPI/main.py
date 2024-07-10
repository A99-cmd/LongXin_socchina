# Imports

## std
import json
import queue
import time
## 3p
import paho.mqtt.client as mqtt
## project
from app import LongxinApp



# message
q_ali = queue.Queue()

# data
x = 0

deadcount = [0, 0, 0, 0]

# NETWORK----MQTT
PORT=1883
HOST=r"iot-06z00itmv0vajzw.mqtt.iothub.aliyuncs.com"
DEV_ID = r"jk3pVO4Uxky.loongnixpi|securemode=2,signmethod=hmacsha256,timestamp=1720512988338|" 
PRO_ID = r"loongnixpi&jk3pVO4Uxky" 
AUTH_INFO = r"151c3caa49efaf8332f60dcefeefef9dafbeac280d7fd61ff35d66b1b7967b20"

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(r"/sys/jk3pVO4Uxky/loongnixpi/thing/service/property/set")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    # print(msg.topic)
    str = json.loads(msg.payload)
    print(str)
    q_ali.put(str)
   

def update_data():
    print("List:" + str(q_ali.qsize()))
    if q_ali.qsize() == 0:
        deadcount[0] = deadcount[0] + 1
        deadcount[1] = deadcount[1] + 1
        deadcount[2] = deadcount[2] + 1
        deadcount[3] = deadcount[3] + 1
    while(q_ali.qsize()): 
        message = q_ali.get()
        a = app.update_message(message=message)
        if a == 0:
            deadcount[0] = deadcount[0] + 1
            deadcount[1] = deadcount[1] + 1
            deadcount[2] = deadcount[2] + 1
            deadcount[3] = deadcount[3] + 1
        else:
            deadcount[a - 1] = 0

    if deadcount[0] >= 10:
        app.button1.configure(text="NODE1\nClosed", bg=app.color[5], anchor="center")
    if deadcount[1] >= 10:
        app.button2.configure(text="NODE2\nClosed", bg=app.color[5], anchor="center")
    if deadcount[2] >= 10:
        app.button3.configure(text="NODE3\nClosed", bg=app.color[5], anchor="center")
    if deadcount[3] >= 10:
        app.button4.configure(text="NODE4\nClosed", bg=app.color[5], anchor="center")  
    app.after(1000, update_data)


def update_graph():
    if app.graph_opened ==True:
        global x
        x = x + 20
        y = 0
        if app.numb == 0:
            y = app.Temperature1
        if app.numb == 1:
            y = app.Humidity1
        if app.numb == 2:
            y = app.Smoke1
        if app.numb == 3:
            y = app.Pressure1
            
        app.new_graph.update(x=x,y=y)
        if x >= 400:
            x, y = 0, 0
            app.new_graph.x, app.new_graph.y = 0, 0
            app.new_graph.canvas.delete("myline")

    app.after(1000, update_graph)


def update_warning():
    if app.Smoke1 > 35.0:
        app.button1.configure(text="NODE1\nWarning", bg=app.color[5], anchor="center")
    if app.Smoke2 > 35.0:
        app.button2.configure(text="NODE2\nWarning", bg=app.color[5], anchor="center")
    if app.Smoke3 > 35.0:
        app.button3.configure(text="NODE3\nWarning", bg=app.color[5], anchor="center")
    if app.Smoke4 > 35.0:
        app.button4.configure(text="NODE4\nWarning", bg=app.color[5], anchor="center")    
    app.after(5000, update_warning)

if __name__ == '__main__':
    client = mqtt.Client(DEV_ID)
    
    client.on_connect = on_connect
    client.on_message = on_message
    
    client.username_pw_set(PRO_ID,AUTH_INFO)
    client.connect(HOST, PORT, 90)
    client.loop_start()


    app = LongxinApp()
    app.after(1000, update_data)
    app.after(1000, update_graph)
    app.after(1000, update_warning)
    app.mainloop()
    

    while True:
        pass




