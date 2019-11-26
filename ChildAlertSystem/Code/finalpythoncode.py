# -*- coding: utf-8 -*-
"""
Created on Mon Nov 11 13:39:12 2019

@author: Trishok
"""
from __future__ import print_function
import sys
import ssl
import time
import os
import logging, traceback
import paho.mqtt.client as mqtt
import socket
fileDir = os.path.dirname(os.path.realpath('__file__'))

IoT_protocol_name = "x-amzn-mqtt-ca"
aws_iot_endpoint = "a2gw161u0ey957-ats.iot.us-west-2.amazonaws.com" # <random>.iot.<region>.amazonaws.com
url = "https://{}".format(aws_iot_endpoint)
ca = os.path.join(fileDir, 'VeriSign-Class-3-Public-Primary-Certification-Authority-G5.pem')
cert = os.path.join(fileDir, 'a407f17540-certificate.pem') 
private = os.path.join(fileDir, 'a407f17540-private.pem')
logger = logging.getLogger()
logger.setLevel(logging.DEBUG)
handler = logging.StreamHandler(sys.stdout)
log_format = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
handler.setFormatter(log_format)
logger.addHandler(handler)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)         

s.bind(('192.168.1.3', 80))
s.listen(0)

def ssl_alpn():
    try:
        #debug print opnessl version
        logger.info("open ssl version:{}".format(ssl.OPENSSL_VERSION))
        ssl_context = ssl.create_default_context()
        ssl_context.set_alpn_protocols([IoT_protocol_name])
        ssl_context.load_verify_locations(cafile=ca)
        ssl_context.load_cert_chain(certfile=cert, keyfile=private)
        return  ssl_context
    except Exception as e:
        print("exception ssl_alpn()")
        raise e
if __name__ == '__main__':
    topic = "ECE574_Project"
    try:
        mqttc = mqtt.Client()
        ssl_context= ssl_alpn()
        mqttc.tls_set_context(context=ssl_context)
        logger.info("start connect")
        mqttc.connect(aws_iot_endpoint, port=443)
        logger.info("connect success")
        mqttc.loop_start()
        message = "Hey you left you Child in the Car please Hurry"
        msgflag = False
        while True:
            client, addr = s.accept()
            print("Got a connection from %s" % str(addr))
            if client:
                content = client.recv(32)
                alertval=int(content)
                if alertval == 2:
                   print("Child alert:")
                   mqttc.publish(topic, message)
                   #print("Closing connection")
                   #client.close()
            time.sleep(1)

    except Exception as e:
        logger.error("exception main()")
        logger.error("e obj:{}".format(vars(e)))
        logger.error("message:{}".format(e.message))
        traceback.print_exc(file=sys.stdout)
