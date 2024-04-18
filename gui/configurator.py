#!/usr/bin/python3
# -*- coding: utf-8 -*-
from PyQt5 import QtWidgets, uic
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QFileDialog, QDialog
from PyQt5.QtCore import QTimer,QDateTime, QFile, QTextStream, Qt
from PyQt5.QtGui import QFont

import sys
import json
import random
import argparse
import datetime
import os
import time
import colorsys
import traceback
import threading

from pathlib import Path


def ioModeFromString(instr):
    #define NOTUSED 0
    #define DI_INPUT_PULLUP 1          // Will return 1 when button is pressed, going low to ground
    #define DI_INPUT_FLOATING 2        //
    #define DI_INPUT_STEP 3            // Will step increase a value while holding the button presed
    #define DO_BOOL 129                // digital out 1 or 0
    #define AI_RAW 4                   // Use only with main flight control, this will most likely update every frame
    #define AI_FILTER 5                // Use this, Noize and deadband filtered to avoid lot of communication spam.
    #define AI_OVERSAMPLE 6            // TODO // Read the value multiple times to fake higher resolution, for main flight controls that have limited use of potensoimeter
    #define AO_PWM 130                 // TODO // Only on PWM pins
    #define AO_DAC 131                 // TODO // Only on DUE boards
    #define AO_SERVO 132               // TODO
    #define DI_ROTARY_ENCODER_TYPE1 10 //
    #define DO_HIGH 133                // Use with care, do not load more than 20mA on pin!
    #define DO_LOW 134                 // Use with care, do not load more than 20mA on pin!
    #define USED 13                    // Used by functions that need extra pins, not sure if we need this or just use 0
    #define DI_3WAY 14                 // TODO 3-way switch
    #define DI_3WAY_2 15               // 3 way switch with special function, uses 4 wires
    #define DO_TM1637_DEC 135          // Display decimal number
    #define DI_4X4 16                  // 4x4 key matrix
    #define AO_STEPPER 136             // Stepper motor
    #define AO_TEXT 137                // Raw text output
    #define MCP_DI 17
    #define MCP_DO 138
    if instr == "NOTUSED":
        return 0
    if instr == "DI_INPUT_PULLUP":
        return 1
    if instr == "DI_INPUT_FLOATING":
        return 2
    if instr == "DI_INPUT_STEP":
        return 3
    if instr == "DO_BOOL":
        return 129
    if instr == "AI_RAW":
        return 4
    if instr == "AI_FILTER":
        return 5
    if instr == "AI_OVERSAMPLE":
        return 6
    if instr == "AO_PWM":
        return 130
    if instr == "AO_DAC":
        return 131
    if instr == "AO_SERVO":
        return 132
    if instr == "DI_ROTARY_ENCODER_TYPE1":
        return 10
    if instr == "DO_HIGH":
        return 133
    if instr == "DO_LOW":
        return 134
    if instr == "USED":
        return 13
    if instr == "DI_3WAY":
        return 14
    if instr == "DI_3WAY_2":
        return 15
    if instr == "DO_TM1637_DEC":
        return 135
    if instr == "DI_4X4":
        return 16
    if instr == "AO_STEPPER":
        return 136
    if instr == "AO_TEXT":
        return 137
    if instr == "MCP_DI":
        return 17
    if instr == "MCP_DO":
        return 138
    return 0
    
def ioModeFromInt():
    return 0
    
class ConfLine(QWidget):
    def __init__(self,line, parent=None):
        super().__init__()
        self.ui = uic.loadUi(os.path.join(current_dir, "line.ui"), self))
        self.parent = parent
        self.rawLine = line
        self.known = False
        self.master = "1"
        self.slave = "0"
        self.ioMode = "NOTUSED"
        self.pinName = "D0"
        self.pinExtra = "0"
        
        self.known = self.readLine(line)
        if self.known:
            self.parent.addWidget2(self)
    
    def readLine(self, line):
        if len(line) > 0:
            if line[0] == '#' or line[0] == '/' or line[0] == '*':
                return False
            else:
                print("config line", line)
                sline = line.split(";")
                print(len(sline))
                if len(sline) == 13:
                    msp = sline[0].split(".")
                    if (len(msp) == 3):
                        self.master = msp[0]
                        self.slave = msp[1]
                        self.pinName = msp[2]
                    self.pinExtra = sline[1]
                    self.ioMode = sline[2]
                    self.invert = sline[3]
                    self.center = sline[4]
                    self.min = sline[5]
                    self.max = sline[6]
                    self.dataref = sline[7]
                    self.xpcenter = sline[8]
                    self.xpmin = sline[9]
                    self.xpmax = sline[10]
                    self.xpexp = sline[11]
                    self.commentAndLinebreak = sline[12]
                    
                    self.ui.lineEditMaster.setText(self.master)
                    self.ui.lineEditSlave.setText(self.slave)
                    self.ui.lineEditPin.setText(self.pinName)
                    self.ui.lineEditPinExtra.setText(self.pinExtra)
                    self.ui.lineEditIOMode.setText(self.ioMode)
                    self.ui.lineEditInverted.setText(self.invert)
                    self.ui.lineEditCenter.setText(self.center)
                    self.ui.lineEditMin.setText(self.min)
                    self.ui.lineEditMax.setText(self.max)
                    self.ui.lineEditDataref.setText(self.dataref)
                    self.ui.lineEditXpCenter.setText(self.xpcenter)
                    self.ui.lineEditXpMin.setText(self.xpmin)
                    self.ui.lineEditXpMax.setText(self.xpmax)
                    self.ui.lineEditXpExp.setText(self.xpexp)
                    
                    self.ui.lineEditDataref.sizeHint()
                return True
        return False
        
    def readUIValues(self):
        self.master = self.ui.lineEditMaster.text()
        self.slave = self.ui.lineEditSlave.text()
        self.pinName = self.ui.lineEditPin.text()
        self.pinExtra = self.ui.lineEditPinExtra.text()
        self.ioMode = self.ui.lineEditIOMode.text()
        self.invert = self.ui.lineEditInverted.text()
        self.center = self.ui.lineEditCenter.text()
        self.min = self.ui.lineEditMin.text()
        self.max = self.ui.lineEditMax.text()
        self.dataref = self.ui.lineEditDataref.text()
        self.xpcenter = self.ui.lineEditXpCenter.text()
        self.xpmin = self.ui.lineEditXpMin.text()
        self.xpmax = self.ui.lineEditXpMax.text()
        self.xpexp = self.ui.lineEditXpExp.text()
        
    def getSaveLine(self):
        self.readUIValues()
        if self.known:
            out = ""
            out += str(self.master) + "." + str(self.slave) + "." +str(self.pinName) + ";"
            out += str(self.pinExtra) + ";"
            out += str(self.ioMode) + ";"
            out += str(self.invert) + ";"
            out += str(self.center) + ";"
            out += str(self.min) + ";"
            out += str(self.max) + ";"
            out += str(self.dataref) + ";"
            out += str(self.xpcenter) + ";"
            out += str(self.xpmin) + ";"
            out += str(self.xpmax) + ";"
            out += str(self.xpexp) + ";"
            out += self.commentAndLinebreak + "\n"
            
        else:
            out = self.rawLine + "\n"
        
        return out
        
    def getmaster(self):
        return str(self.master)
        
    def getslave(self):
        return str(self.slave)    
        
    def getioMode(self):
        return str(self.ioMode)    
        
    def getpinName(self):
        return str(self.pinName) 
        
    def getpinExtra(self):
        return str(self.pinExtra) 
        
class RunGUI(QMainWindow):
    def __init__(self,):
        super(RunGUI,self).__init__()

        self.masters = []
        self.ioList = []
        self.confLines = []

        
        self.initUI()
        
    def initUI(self):
        #self.root = Tk() # for 2d drawing
        
        
        current_dir = os.path.dirname(os.path.abspath(__file__))
        self.ui = uic.loadUi(os.path.join(current_dir, "configurator.ui"), self)
        
        #self.setGeometry(200, 200, 300, 300)
        #self.resize(640, 480)
        self.setWindowTitle("openSimIO Configurator")
        #self.setWindowFlags(Qt.WindowStaysOnTopHint)
        
        
        self.ui.pushButton.clicked.connect(self.buttonClicked)
        self.ui.actionOpen_config_file.triggered.connect(self.loadConfigFile)
        self.ui.actionSave.triggered.connect(self.saveConfigFile)


        font = QFont("Sans")
        font.setPointSize(12)
        self.setFont(font)
        
        self.timer = QTimer()
        self.timer.timeout.connect(self.loop)
        self.timer.start(100)


    def buttonClicked(self):
        print("button")
        
        
    def loop(self):
        return
        #print("loop")
        
        
    def findMasters(self, data):
        lines = data.split("\n")
        for line in lines:
            if len(line) > 5:
                if line[0] == '/':
                    print("found master", line)
                    
    def findIOLines(self, data):
        lines = data.split("\n")
        for line in lines:
            if len(line) > 0:
                if line[0] == '#' or line[0] == '/':
                    continue
                else:
                    print("config line", line)
                    # newconf = ConfLine(line, parent=self)
                    # self.ioList.append(newconf)
                    
    def createConfigLines(self, data):
        lines = data.split("\n")
        for line in lines:
            self.confLines.append(ConfLine(line, parent=self))
            
    def addWidget2(self, ww):
        self.ui.verticalLayout.addWidget(ww)
        
    
    def saveConfigFile(self):
        
        dialog = QFileDialog(self)
        dialog.setWindowTitle("Save config")
        dialog.setAcceptMode(QFileDialog.AcceptSave)
        dialog.setNameFilter("Config text (*.txt)")
        dialog.setFileMode(QFileDialog.AnyFile)
        dialog.setDefaultSuffix("txt")
        if dialog.exec_() == QDialog.Accepted:
            filename = str(dialog.selectedFiles()[0])
            print(filename)
            
            with open(filename, "w") as outfile:
                for line in self.confLines:
                    out = line.getSaveLine()
                    outfile.write(out)
        
        
    def sendConfig(self):
        #int len = sprintf(out, "{%d;%d;1;%s,%d,%d;}", pins[i].master, pins[i].slave, pins[i].pinNameString, pins[i].ioMode, pins[i].pinExtra);
        out = "{"+master+";"+slave+";1;"+pinName+","+ioMode+","+pinExtra+";}"
        print("send", out)
        
    def parseConfig(self, data):
        self.findMasters(data)
        self.findIOLines(data)
        self.createConfigLines(data)
        
    def loadConfigFile(self):
        dlg = QFileDialog()
        dlg.setFileMode(QFileDialog.AnyFile)
        #dlg.setFilter("Text files (*.txt)")
        #filenames = QStringList()

        if dlg.exec_():
            filenames = dlg.selectedFiles()
            f = open(filenames[0], 'r')
            self.filename = filenames[0]
            with f:
                data = f.read()
                #print(data)
                self.parseConfig(data)
        
        
        
        
if __name__ == "__main__":

    try:
        app = QApplication(sys.argv)
        win = RunGUI()
        win.show()
        sys.exit(app.exec_())
    except Exception as err:
        exception_type = type(err).__name__
        print(exception_type)
        print(traceback.format_exc())
        os._exit(1)
    print ("program end")
    os._exit(0)
