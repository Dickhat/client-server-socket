import socket
import time
import datetime
import sys


sock = socket.socket()

ip = 'empty' 
port = 0

# Получение данных из файла конфигурации
with open('.\\configure.txt', 'r') as conf_file:
    data = conf_file.read().split(':')

    if (len(data) != 2):
        print("\nconfigure file incorrect\n")
        exit 

    if(len(sys.argv) != 2):
        print("\nNo input message\n")
        exit 

    ip = data[0]
    port = int(data[1])

# Подключение к серверу
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    sock.connect((ip, port))

    file = open('ClientLog.txt', 'a')

    # Время подключения к серверу
    cur_time = datetime.datetime.today()
    file.write(f"\n\nConnection {cur_time.day}.{cur_time.month}.{cur_time.year} {cur_time.hour}:{cur_time.minute}:{cur_time.second}")
    file.write(f"\nServer {ip}:{port}")

    #time.sleep(2)

    # Отправка серверу данных
    send_str = sys.argv[1]
    sock.sendall(send_str.encode('UTF-8'))

    # Запись времени отправки
    cur_time = datetime.datetime.today()
    file.write(f"\nMessage send time {cur_time.hour}:{cur_time.minute}:{cur_time.second}")
    file.write(f"\nMessage content: {send_str}")

    # Получение ответа от сервера
    response = sock.recv(4096)
    cur_time = datetime.datetime.today()
    file.write(f"\nMessage recieved time {cur_time.hour}:{cur_time.minute}:{cur_time.second}")
    file.write(f"\nMessage content: {response}")

    file.close()
