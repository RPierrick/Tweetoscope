from kafka import KafkaConsumer

consumerProperties = { "bootstrap_servers":['localhost:9092'], 
                       "auto_offset_reset":"earliest",
                       "group_id":"myOwnPrivatePythonGroup" }

consumer = KafkaConsumer(**consumerProperties)
consumer.subscribe("tweets")
# or: consumer = KafkaConsumer("test", **consumerProperties)

#try:
#   while True:
#      records = consumer.poll(1000)
#      if records is None: continue
#      else:
#         for topicPartition, consumerRecords in records.items():
#            for record in consumerRecords:
#               print("%s" % (record.value.decode()))
#finally:
#   consumer.close()


for record in consumer:
   print("%s" % (record.value.decode()))

