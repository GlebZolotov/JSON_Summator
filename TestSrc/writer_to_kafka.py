from json import loads, dumps
from time import sleep
from pykafka import KafkaClient
from numpy.random import normal
from math import fabs
from generator_without_new_parameters import prob
import sys

client = KafkaClient(hosts="127.0.0.1:9092")
topic = client.topics[sys.argv[1]]
with topic.get_sync_producer() as producer:
    i = 0
    gen = prob()
    for s in gen:
        i += 1
        producer.produce(s.encode('utf-8'))
        print("Send " + loads(s)["rqId"])
        sleep(fabs(normal(1, 1)))
        if i == int(sys.argv[3]):
            break
