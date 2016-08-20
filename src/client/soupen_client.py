#!/usr/bin/python
import socket
HOST='127.0.0.1'
PORT=7788
s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)      
s.connect((HOST,PORT))
##*3\r\n$3\r\nset\r\n$5\r\nhello\r\n$6\r\nsoupen\r\n
while 1:
       cmd=raw_input("Please input cmd:")
       text = cmd.split(' ')
       length = len(text)
       print(length)
       send_text = '*'
       send_text = send_text + str(length)
       send_text = send_text + '\r'
       send_text = send_text + '\n'
       for i in range(0, length):
         send_text = send_text + '$'
         send_text = send_text + str(len(text[i]))
         send_text = send_text + '\r'
         send_text = send_text + '\n'
         send_text = send_text + text[i]
         send_text = send_text + '\r'
         send_text = send_text + '\n'
       s.sendall(send_text)
       data=s.recv(1024)
       print data
s.close()
