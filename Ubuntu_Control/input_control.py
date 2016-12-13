#!/usr/bin/python2.7
#-*- encoding: UTF-8 -*-

'''
这份代码的python版本用的python2.7，另外需要注意：
运行python脚本之前需要提前安装pyserial和threading库
系统需要安装xdotool工具（用来控制鼠标和键盘）
这个代码只能运行在Linux系统，实测Ubuntu16.04

'''

import sys
import os
import time
import serial
import threading
import math
#--------------------------------------全局变量定义---------------------------------------
#电位器
finger1_inflex = 2900         #大于
finger2_inflex = 1000         #小于
finger3_inflex = 1000         #小于
finger4_inflex = 2000         #大于
finger5_inflex = 2000
joint1_min = 2900
joint1_max = 3700
joint2_min = 3000
joint2_max = 3600
joint3_min = 2500
joint3_max = 3200
joint4_min = 20
joint4_max = 110

#陀螺仪
pitch_left_limit = 45
pitch_right_limit = 140
roll_front_limit_min = 280
roll_front_limit_max = 330
roll_back_limit_min = 50
roll_back_limit_max = 120
#yaw_left_limit_min = 50
#yaw_left_limit_max = 150
#yaw_right_limit_min = 300
#yaw_right_limit_max = 350
yaw_left_limit = 160
yaw_right_limit = 100

#手势状态列表下标定义
finger1_stat = 0
finger2_stat = 1
finger3_stat = 2
finger4_stat = 3
finger5_stat = 4
roll_front_stat = 5
roll_back_stat = 6
pitch_left_stat = 7
pitch_right_stat = 8
yaw_left_stat = 9
yaw_right_stat = 10
stat = [0]*11            #手势状态

data_array = [0]*30            #每帧的数据部分有30byte
ser = serial.Serial('/dev/ttyUSB1', 9600)

#-----------------------------------全局变量定义结束-----------------------------------------

def data_process():
    x = 0.0
    try:
        while(True):
            #aacx = (data_array[0] << 8) + data_array[1]
            #aacy = (data_array[2] << 8) + data_array[3]
            #aacz = (data_array[4] << 8) + data_array[5]
            pitch = (data_array[6] << 8) + data_array[7]
            roll = (data_array[8] << 8) + data_array[9]
            yaw = (data_array[10] << 8) + data_array[11]
            finger1 = (data_array[12] << 8) + data_array[13]
            finger2 = (data_array[14] << 8) + data_array[15]
            finger3 = (data_array[16] << 8) + data_array[17]
            finger4 = (data_array[18] << 8) + data_array[19]
            finger5 = (data_array[20] << 8) + data_array[21]
            #joint1 = (data_array[22] << 8) + data_array[23]
            #joint2 = (data_array[24] << 8) + data_array[25]
            #joint3 = (data_array[26] << 8) + data_array[27]
            #joint4 = (data_array[28] << 8) + data_array[29]

            #--------------------------------刷新手势列表值-----------------------------------------
            if(pitch > pitch_right_limit):
                stat[pitch_right_stat] = 1
            else:
                stat[pitch_right_stat] = 0
            if(pitch < pitch_left_limit):
                stat[pitch_left_stat] = 1
            else:
                stat[pitch_left_stat] = 0
            if(roll < roll_back_limit_max and roll > roll_back_limit_min):
                stat[roll_back_stat] = 1
            else:
                stat[roll_back_stat] = 0
            if(roll < roll_front_limit_max and roll > roll_front_limit_min):
                stat[roll_front_stat] = 1
            else:
                stat[roll_front_stat] = 0

            #if(yaw > yaw_left_limit_min and yaw < yaw_left_limit_max):
            if(yaw >yaw_left_limit):
                stat[yaw_left_stat] = 1
            else:
                stat[yaw_left_stat] = 0
            #if(yaw < yaw_right_limit_max and yaw > yaw_right_limit_min):
            if(yaw < yaw_right_limit):
                stat[yaw_right_stat] = 1
            else:
                stat[yaw_right_stat] = 0

            if(finger1 > finger1_inflex):
                stat[finger1_stat] = 1
            else:
                stat[finger1_stat] = 0
            if(finger2 < finger2_inflex):
                stat[finger2_stat] = 1
            else:
                stat[finger2_stat] = 0
            if(finger3 < finger3_inflex):
                stat[finger3_stat] = 1
            else:
                stat[finger3_stat] = 0
            if(finger4 > finger4_inflex):
                stat[finger4_stat] = 1
            else:
                stat[finger4_stat] = 0
            if(finger5 > finger5_inflex):
                stat[finger5_stat] = 1
            else:
                stat[finger5_stat] = 0

            #----------------------------------查手势列表来定义指令--------------------------------------
            if(stat[finger1_stat] == 1):              #鼠标左键
                os.system('xdotool mousedown 1')
            else:
                os.system('xdotool mouseup 1')
            if(stat[finger2_stat] == 0 and stat[finger3_stat] == 1 and stat[finger4_stat] == 1 and stat[finger5_stat] == 1):  #W 游戏前进
                os.system('xdotool keydown --delay 2 w')
            else:
                os.system('xdotool keyup --delay 2 w')
            if(stat[finger2_stat] == 0 and stat[finger3_stat] == 0 and stat[finger4_stat] == 1 and stat[finger5_stat] == 1):    #S 游戏后退
                os.system('xdotool keydown --delay 2 s')
            else:
                os.system('xdotool keyup --delay 2 s')
            if(stat[finger2_stat] == 1 and stat[finger3_stat] == 1 and stat[finger4_stat] == 1 and stat[finger5_stat] == 1):    #空格 游戏跳跃
                os.system('xdotool keydown --delay 2 space')
            else:
                os.system('xdotool keyup --delay 2 space')
            #A 游戏左移
            #if(stat[finger3_stat] == 1 and stat[finger4_stat] == 1 and stat[pitch_left_stat] == 1 and stat[finger2_stat] == 0 and stat[finger5_stat] == 0):
            if(stat[pitch_left_stat]):
                os.system('xdotool keydown --delay 2 a')
            else:
                os.system('xdotool keyup --delay 2 a')
            #D 游戏右移
            #if(stat[finger3_stat] == 1 and stat[finger4_stat] == 1 and stat[pitch_right_stat] == 1 and stat[finger2_stat] == 0 and stat[finger5_stat] == 0):
            if(stat[pitch_right_stat]):
                os.system('xdotool keydown --delay 2 d')
            else:
                os.system('xdotool keyup --delay 2 d')
            #Q 游戏换武器
            #if(stat[finger4_stat] == 1 and stat[finger2_stat] == 0 and stat[finger3_stat] == 0 and stat[finger5_stat] == 0):
            #    os.system('xdotool key q')
            if(stat[roll_front_stat] == 1):          #鼠标上移
                os.system('xdotool mousemove_relative --sync 0 40')
            if(stat[roll_back_stat] == 1):           #鼠标下移
                os.system('xdotool mousemove_relative --sync -- 0 -40')
            #鼠标左移
            #if(stat[finger4_stat] == 1 and stat[pitch_left_stat] == 1 and stat[finger2_stat] == 0 and stat[finger3_stat] == 0 and stat[finger5_stat] == 0):
            if(stat[yaw_left_stat] == 1):
                os.system('xdotool mousemove_relative --sync -- -40 0')
            #鼠标右移
            #if(stat[finger4_stat] == 1 and stat[pitch_right_stat] == 1 and stat[finger2_stat] == 0 and stat[finger3_stat] == 0 and stat[finger5_stat] == 0):
            if(stat[yaw_right_stat] == 1):
                os.system('xdotool mousemove_relative --sync 40 0')

        time.sleep(0.02)
    except KeyboardInterrupt:
        print
        print "Interrupted by user, shutting down"
        sys.exit(0)

def read_data():
    try:
        while(True):
            start = ord(ser.read())
            if(start == 0x88):                   #检查帧头
                fun = ord(ser.read())
                if(fun == 0xa1):               #检查功能码
                    length = ord(ser.read())
                    for i in range(length):     #开始接收数据
                        data_array[i] = ord(ser.read())
                    checksum = ord(ser.read())
                    #if(checksum != length + 3):     #检查校验和是否正确
                    #    continue
                else:
                    #print("fun code is wrong!")
                    continue
            else:
                #print("start code is wrong!")
                continue
            time.sleep(0.03)
    except KeyboardInterrupt:
        print
        print "Interrupted by user, shutting down"
        sys.exit(0)
funcs = [read_data, data_process]
nfuncs = range(len(funcs))

def main():

    threads = []
    try:
        for i in nfuncs:
            t = threading.Thread(target = funcs[i])
            threads.append(t)
        for i in nfuncs:
            threads[i].start()
        for i in nfuncs:
            threads[i].join()

    except KeyboardInterrupt:
        print
        print "Interrupted by user, shutting down"
        sys.exit(0)
if __name__ == "__main__":
    main()
