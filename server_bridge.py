#!/bin/python3
import socket,sys,signal
import paho.mqtt.client as mqtt_client
import threading, queue

# Socket for sensors
HOST = 'localhost'
PORT = 1700

# MQTT Broker
BROKER_IP = '127.0.0.1'
BROKER_PORT = 1883
CLIENT_NAME = 'Sensors Bridge'
BROKER_USER = 'mqtt_user'
BROKER_PASS = 'mqtt_passwd'
APP_NAME = 'DHT11WiFi'

# message exchange queue
q = queue.Queue()


def signal_handler(sig,frame):
	print('Closing bridge...')
	sys.exit(0)

def on_publish(client, userdata, result):
	print('data published\n')
	pass

def on_connect(client, userdata, flags, rc):
    if rc==0:
        print("connected OK Returned code=",rc)
    else:
        print("Bad connection Returned code=",rc)

def mqtt_thread():
	client = mqtt_client.Client(CLIENT_NAME)
	client.on_publish = on_publish
	client.on_connect = on_connect
	client.loop_start()
	client.username_pw_set(BROKER_USER, BROKER_PASS)
	client.connect(BROKER_IP, BROKER_PORT)
	while True:
		msg = q.get().decode()
		div = msg.find('\r')
		dev = APP_NAME + '/' + msg[0:(div)]
		data = msg[(div+2):]
		print(dev)
		print('------')
		print(data)
		print('======')
		client.publish(dev, data)
		q.task_done()

	client.loop_stop()

def main():
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		s.bind((HOST, PORT))
		s.listen()
		while True:
			conn, addr = s.accept()
			with conn:
				conn.settimeout(5)
				print('Connection from ', addr)
				data = b''
				end = False
				while not end:
					try:
						data += conn.recv(1024)
					except:
						end = True
				conn.send(str(len(data)).encode())
				q.put(data)


if __name__ == '__main__':
	print('Starting bridge.')
	signal.signal(signal.SIGINT, signal_handler)
	mqttThread = threading.Thread(target=mqtt_thread, daemon=True)
	mainThread = threading.Thread(target=main, daemon=True)
	mqttThread.start()
	mainThread.start()
	mainThread.join()
	mqttThread.join()
	q.join()
