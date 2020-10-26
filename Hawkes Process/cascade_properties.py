from kafka import KafkaConsumer

consumerProperties = { "bootstrap_servers":['localhost:9092'], 
                       "auto_offset_reset":"earliest",
                       "group_id":"myOwnPrivatePythonGroup" }

consumer = KafkaConsumer(**consumerProperties)
consumer.subscribe("cascade_properties")

for record in consumer:
   print("%s" % (record.value.decode()))
