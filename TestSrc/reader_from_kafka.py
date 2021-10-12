from json import loads
from pykafka import KafkaClient
import sys

client = KafkaClient(hosts="127.0.0.1:9092")
topic = client.topics['input_topic']
consumer = topic.get_simple_consumer()
while True:
    msg = consumer.consume()
    print(loads(msg.value)["rqId"])
