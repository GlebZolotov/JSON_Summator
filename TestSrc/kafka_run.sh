#!/usr/bin/bash

# run: ./kafka_run.sh <count_of_topics> <count_of_writers> <count_of_messages> <count_of_workers>
# get path to kafka-folder
path_to_kafka=$( cd "$(dirname $0)/../kafka" || return; pwd)
# start terminals with commands
#gnome-terminal -- bash -c "${path_to_kafka}/bin/zookeeper-server-start.sh ${path_to_kafka}/config/zookeeper.properties"
#sleep 5
#gnome-terminal -- bash -c "${path_to_kafka}/bin/kafka-server-start.sh ${path_to_kafka}/config/server.properties"
#sleep 10
#gnome-terminal -- bash -c "${path_to_kafka}/bin/kafka-server-start.sh ${path_to_kafka}/config/server.properties"
if [ "$1" -gt 1 ]; then
    #gnome-terminal -- bash -c "${path_to_kafka}/bin/kafka-topics.sh --create --topic input_topic1 --bootstrap-server localhost:9092"
    #gnome-terminal -- bash -c "${path_to_kafka}/bin/kafka-topics.sh --create --topic output_topic1 --bootstrap-server localhost:9092"
    #gnome-terminal -- bash -c "${path_to_kafka}/bin/kafka-topics.sh --create --topic input_topic2 --bootstrap-server localhost:9092"
    #gnome-terminal -- bash -c "${path_to_kafka}/bin/kafka-topics.sh --create --topic output_topic2 --bootstrap-server localhost:9092"
    # start python scripts
    gnome-terminal -- bash -c "cd $(dirname $0); pipenv run python reader_from_kafka.py output_topic1 $2 $3"
    gnome-terminal -- bash -c "cd $(dirname $0); pipenv run python reader_from_kafka.py output_topic2 $2 $3"
    sleep 2
    #gnome-terminal -- bash -c "cd $(dirname $0)/../build; ./main -b localhost:9092 -i input_topic1 -i input_topic2 -o output_topic1 -o output_topic2 -g test1 -g test2 -n $4 -s 100 -t 20 -l 50 -c ../TestSrc/csv/data0.csv; exec bash"
    #sleep 2
    for (( i=1; i <= $2; i++ ))
    do
        gnome-terminal -- bash -c "cd $(dirname $0); pipenv run python writer_to_kafka.py input_topic1 $i $3"
        gnome-terminal -- bash -c "cd $(dirname $0); pipenv run python writer_to_kafka.py input_topic2 $i $3"
    done
else
    gnome-terminal -- bash -c "${path_to_kafka}/bin/kafka-topics.sh --create --topic input_topic --bootstrap-server localhost:9092"
    gnome-terminal -- bash -c "${path_to_kafka}/bin/kafka-topics.sh --create --topic output_topic --bootstrap-server localhost:9092"
    # start python scripts
    #gnome-terminal -- bash -c "cd $(dirname $0); pipenv run python reader_from_kafka.py output_topic $2 $3; exec bash"
    #sleep 2
    gnome-terminal -- bash -c "cd $(dirname $0)/../build; touch ../TestSrc/csv/update.txt; ./main -b localhost:9092 -i input_topic -o output_topic -g test1 -n $4 -s 100 -t 20 -l 50 -c ../TestSrc/csv; exec bash"
    #sleep 2
    for (( i=1; i <= $2; i++ ))
    do
        gnome-terminal -- bash -c "cd $(dirname $0); pipenv run python writer_to_kafka.py input_topic $i $3"
    done
fi

# Templates
# ./main -b localhost:9092 -i input_topic1 -i input_topic2 -o output_topic1 -o output_topic2 -g test1 -g test2 -n 4 -s 100 -t 20 -l 50 -c ../TestSrc/csv/data0.csv
#../kafka/bin/kafka-topics.sh --delete --zookeeper localhost:2181 --topic output_topic
#./TestSrc/kafka_run.sh 1 2 20 1
