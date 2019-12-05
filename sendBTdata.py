#import bluetooth
from bluetooth import *
import time
BTAddress1 = "98:D3:71:FD:A5:63"
BTAddress2 = "50:51:A9:FE:33:2F"
#"98:D3:91:FD:97:F9"

def sendMessage(data):
	port = 1
	sock=bluetooth.BluetoothSocket( bluetooth.RFCOMM )
	sock.connect((BTAddress1, port))
	sock.send(data)
	sock.close()

def receiveMessage():
	port = 1
	sock=bluetooth.BluetoothSocket( bluetooth.RFCOMM )
	sock.connect((BTAddress1, port))
	data = sock.recv(10)
	print "received [%s]" % data
 	sock.close()

port = 1
sock = BluetoothSocket(RFCOMM)
sock.connect((BTAddress2,port))


while True:
	#sendMessage("1")
	#time.sleep(1)
	#receiveMessage()
	#time.sleep(1)
	#sendMessage("2")
	#time.sleep(1)
	#receiveMessage()
	sock.send("1")
	#time.sleep(0.1)
	

	data = sock.recv(10)
	sock.send(data)
	

	#print data
	#time.sleep(1.5)
        #sock.send("2")
	#time.sleep(0.1)
        #data = sock.recv(10)
        #print data
        #time.sleep(1.5)
