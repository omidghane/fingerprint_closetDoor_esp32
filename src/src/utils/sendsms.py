import requests

def send_sms(call_number, msg):
    url = "https://niksms.com/fa/publicapi/ptpsms"
    username = "?username=09304110346"
    password = "&password=MilanVazir7"
    senderNumber = "&senderNumber=500044778008"

    msg = msg

    headers = {
        'Content-Type': 'application/json'
    }

    params = {
        'username': '09304110346',
        'password': "MilanVazir7",
        'senderNumber': "500044778008",
        'message': msg,
        'numbers': call_number
    }

    numbers = "&numbers=" + call_number

    FinalURL = url + username + password + numbers + senderNumber + msg

    response = requests.get(url, headers=headers, params=params)
