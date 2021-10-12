#!/usr/bin/bash

# get path to kafka-folder
path_to_kafka=$( cd "$(dirname $0)/../kafka" || return; pwd)
# construct commands for start kafka
start_zookeeper="${path_to_kafka}/bin/zookeeper-server-start.sh ${path_to_kafka}/config/zookeeper.properties"
start_broker="${path_to_kafka}/bin/kafka-server-start.sh ${path_to_kafka}/config/server.properties"
create_i1_topic="${path_to_kafka}/bin/kafka-topics.sh --create --topic input_topic --bootstrap-server localhost:9092"
create_o1_topic="${path_to_kafka}/bin/kafka-topics.sh --create --topic output_topic --bootstrap-server localhost:9092"
# start terminals with commands
gnome-terminal -- bash -c "${start_zookeeper}"
sleep 5
gnome-terminal -- bash -c "${start_broker}"
sleep 5
gnome-terminal -- bash -c "${create_i1_topic}"
sleep 5
gnome-terminal -- bash -c "${create_o1_topic}"
sleep 5
# start python scripts
