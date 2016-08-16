#!/usr/bin/env python

import threading
import collections

class CommandQuee():
    def __init__(self):
        self.lock = threading.RLock()
        self.commands = collections.deque()
        self.event = threading.Event();
        
    def WaitEvent(self):
        self.event.wait()  
        
    def HaveEvent(self):
        return self.event.isSet()    
        
    def Write(self, command, close = True):
        self.lock.acquire()
        self.commands.append(command)
        if (close):
            self.event.set()
        self.lock.release()
        
    def Read(self):
        self.lock.acquire()
        if (len(self.commands) > 0):
            command = self.commands.popleft()
        else:
            command = False
        if (len(self.commands) == 0):
            self.event.clear()
        self.lock.release()
        return command
    
class DataSource():
    def __init__(self, size):
        self.lock = threading.RLock()
        self.data = []
        for i in range(size):
            self.data.append((0))
        
    def Read(self, index):
        self.lock.acquire()
        ret = self.data[index]
        if (type(ret) == type(True)):
            print ret
        self.lock.release()
        return ret
    
    def Write(self, data, index):
        self.lock.acquire()
        self.data[index] = data
        self.lock.release()
        
    