from json import loads, dumps
from pykafka import KafkaClient
import sys

client = KafkaClient(hosts="127.0.0.1:9092")
topic = client.topics[sys.argv[1]]
consumer = topic.get_simple_consumer(consumer_group="test", auto_commit_enable=True)
while True:
    msg = consumer.consume()
    resjson = loads(msg.value)
    print(resjson["rqId"] + " " + dumps(resjson["weights"]))
