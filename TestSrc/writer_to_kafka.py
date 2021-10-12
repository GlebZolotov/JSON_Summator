from json import load, dumps
from time import sleep
from pykafka import KafkaClient
from numpy.random import normal
from math import fabs
import sys

with open("example_input.json", "r") as read_file:
    data = load(read_file)

client = KafkaClient(hosts="127.0.0.1:9092")
topic = client.topics['input_topic']
with topic.get_sync_producer() as producer:
    for i in range(int(sys.argv[2])):
        data["rqId"] = str(sys.argv[1]) + "_" + str(i)
        producer.produce(dumps(data).encode('utf-8'))
        sleep(fabs(normal(1, 1)))
