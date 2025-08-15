import logging
import json
from django.core.management.base import BaseCommand
from accounts.models import Temprature, DeviceFinger  # Note: corrected spelling
import pika

# Setup basic configuration for logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

class Command(BaseCommand):
    help = 'Starts the AMQP subscriber'

    def handle(self, *args, **options):
        connection = pika.BlockingConnection(
            pika.ConnectionParameters(host='192.168.4.15', credentials=pika.PlainCredentials('microservice', 'eGxULCXEArQM25D7g4YfNZWb93kuFB'))
        )
        channel = connection.channel()

        # Ensure that the queue exists
        channel.queue_declare(queue='arduino.temperature', durable=True)
        
        def callback(ch, method, properties, body):
            try:
                payload = json.loads(body.decode('utf-8'))
                try:
                    device = DeviceFinger.objects.get(mac_id=payload['mac_id'])
                    Temprature.objects.create(
                        device=device,
                        humidity=payload['localHum'],
                        temprature=payload['localTemp'],
                        action_time=payload['action_time']
                    )
                    logging.info("Temperature record saved successfully.")
                except DeviceFinger.DoesNotExist:
                    logging.error(f"Device with MAC ID {payload['mac_id']} not found.")
                except Exception as e:
                    logging.error(f"Error saving temperature data: {str(e)}")
            except json.JSONDecodeError:
                logging.error("Error decoding JSON from the message payload")

        channel.basic_consume(queue='arduino.temperature', on_message_callback=callback, auto_ack=True)

        logging.info('Starting to consume')
        channel.start_consuming()

