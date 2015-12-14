#! /usr/bin/env python
# -*- coding: utf-8 -*-

from socket import *
import os
import struct
import sys

g_file_name = None

def sendFile(addr, filename):
    BUFSIZE = 1024
    FILEINFO_SIZE=struct.calcsize('128s32sI8s')

    sendSock = socket(AF_INET,SOCK_STREAM)
    sendSock.connect(addr)
    filename_tosend = filename
    if '\\' in filename_tosend:
        filename_tosend = filename_tosend[filename_tosend.rindex('\\') + 1:]
    print 'file name to send: %s, os.pathsep: %s' % (filename_tosend, os.pathsep)
    fhead=struct.pack('128s11I',filename_tosend,0,0,0,0,0,0,0,0,os.stat(filename).st_size,0,0)
    sendSock.send(fhead)
    
    fp = open(filename,'rb')
    filesize = 0
    print 'Send File Start... '
    while 1:
    
        filedata = fp.read(BUFSIZE)
    
        if not filedata: 
            break
    
        sendSock.send(filedata)
    
    print "Send File End !!! "
    fp.close()
    
    recvResponse(sendSock)
    
    sendSock.close()
    print "Connection Closed!!! "
    
def recvResponse(sock):
    BUFSIZE = 1024
    FILEINFO_SIZE=struct.calcsize('128s32sI8s')
    fhead = sock.recv(FILEINFO_SIZE)
    filename,temp1,filesize,temp2=struct.unpack('128s32sI8s',fhead)
    print filename, filesize
    
    filename = g_file_name
    fp = open(filename,'wb')
    restsize = filesize
    
    print "Receive File Start!!!"  
    while 1:
        if restsize > BUFSIZE:
            filedata = sock.recv(BUFSIZE)
        else:
            filedata = sock.recv(restsize)
    
        if not filedata: 
            break
    
        fp.write(filedata)
        restsize = restsize - len(filedata)
        
        if restsize == 0:
            break
    
    print "Receive File End!!!"  
    fp.close()
         

#linzj
if __name__ == '__main__':
    #addr = ('100.84.43.103',8000)
    addr = (sys.argv[1], int(sys.argv[2]))
    filename = sys.argv[3]
    g_file_name = sys.argv[4]
    sendFile(addr, filename);
    sys.exit(0)
