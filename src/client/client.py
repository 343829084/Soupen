#!/usr/bin/python
import socket
HOST='127.0.0.1'
PORT=7788
s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)      
s.connect((HOST,PORT))
while 1:
       cmd=raw_input("Please input cmd:")
       text = cmd.split(' ')
       length = len(text)
       send_text = '*'
       send_text = send_text + str(len(text[0]))
       send_text = send_text + '('
       send_text = send_text + ')'
       send_text = send_text + text[0]
       send_text = send_text + '('
       send_text = send_text + ')'
       send_text = send_text + str(len(text) - 1)
       send_text = send_text + '('
       send_text = send_text + ')'
       for i in range(1, length):
         send_text = send_text + str(len(text[i]))
         send_text = send_text + '('
         send_text = send_text + ')'
         send_text = send_text + text[i]
         send_text = send_text + '('
         send_text = send_text + ')'
       send_text += "YEDI"
       s.sendall(send_text)
       data=s.recv(1024)
       print data
s.close()