# -*- coding: utf-8 -*-
"""
Created on Mon Nov 11 13:39:12 2019

@author: Trishok
"""
from __future__ import print_function
import sys
import ssl
import time
import random
import queue
import datetime
import os
import logging, traceback
import paho.mqtt.client as mqtt
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
DriverSeatSensrVal=queue.Queue(maxsize=10)
ChildSeatSensrVal=queue.Queue(maxsize=10)

for x in range(10):
    DriverSeatSensrVal.put(255)
    #print(DriverSeatSensrVal.get())
for x in range(10):
    ChildSeatSensrVal.put(0)
    #print(ChildSeatSensrVal.get())
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
            now = datetime.datetime.now().strftime('%Y-%m-%dT%H:%M:%S')
            sens1 = random.choice([0, 0, 0, 0, 0]) # TODO Driver Weight shoul be Read from Api GetDriverWeightSensorVal
            sens2 = random.choice([0, 4, 8, 10, 15]) # TODO Driver Weight shoul be Read from Api GetChildWeightSensorVal
            DriverSeatSensrVal.get(sens1)
            DriverSeatSensrVal.put(sens1)
            ChildSeatSensrVal.get(sens2)
            ChildSeatSensrVal.put(sens2)
            DriverseatSensValAvg=(int(sum(DriverSeatSensrVal.queue)/len(DriverSeatSensrVal.queue)))
            ChildseatSensValAvg=(int(sum(ChildSeatSensrVal.queue)/len(ChildSeatSensrVal.queue)))
            print("DrvAvg && ChildAvg",DriverseatSensValAvg , ChildseatSensValAvg )
            if DriverseatSensValAvg == 0 and ChildseatSensValAvg != 0:
                if msgflag == False:
                    print("Child alert")
                    mqttc.publish(topic, message)
                    msgflag = True
            time.sleep(1)

    except Exception as e:
        logger.error("exception main()")
        logger.error("e obj:{}".format(vars(e)))
        logger.error("message:{}".format(e.message))
        traceback.print_exc(file=sys.stdout)
