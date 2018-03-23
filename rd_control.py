#!/usr/bin/python

# rd_control.py - read from GPIO pins on pi 0, then send
# commands to mpg123 or rd depending on button pushed.
# Used to control:
#	PLAY/PAUSE (button 1) controls mpg123
#	REW (button 2) controls rd
#	FORWARD (button 3) controls rd

# Hardware: I have 3 N.O. push buttons used for control
# Button #1 is tied from pin 40 of the GPIO port to ground
# This is the Play/Pause button (N.O. - Normally Open)

# Button #2 is tied from pin 38 to ground. This is also N.O.
# and is the REWIND control

# Button #3 is tied from pin 36 to ground. This is also N.O.
# This is the NEXT song control.

# Part of the rd.c program to read status from and control mpg123

# ktheis <theis.kurt@gmail.com>
# 22 Mar 2018 


import sys
import os
import subprocess
import time
import RPi.GPIO as GPIO
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)

# set up buttons
GPIO.setup(40, GPIO.IN, pull_up_down=GPIO.PUD_UP) # PLAY/PAUSE (as it says)
GPIO.setup(38, GPIO.IN, pull_up_down=GPIO.PUD_UP) # REWIND (replay current song from start)
GPIO.setup(36, GPIO.IN, pull_up_down=GPIO.PUD_UP) # FORWARD (next song)

# set up LED
# GPIO.setup(37,GPIO.OUT)

# start rd
rd_proc =  subprocess.Popen(['./rd','&'], shell=True)
rd_pid = rd_proc.pid
# use rd_proc.terminate() to close the process
print("Starting rd")
time.sleep(1)

# start mpg123
# I wanted to use subprocess.Popen here but the command line is too complex for it
os.system('mpg123 --fifo /tmp/mpgfifo -R abcd >> /tmp/mpgout &')
print("Starting mpg123")
time.sleep(1)


# initial conditions
with open("/tmp/mpgfifo","a") as mpgfp:
	mpgfp.write("SILENCE\n")  # don't spit out lots of playback messages
	time.sleep(0.1)
	mpgfp.write("SILENCE\n")  # do it twice to flush buffer garbage
	time.sleep(0.1)

while True:
	# Short press: Play/Pause   Long press: Exit and close children
	if GPIO.input(40) == 0: # switch is active LOW, normally HIGH
		with open("/tmp/mpgfifo","a") as mpgfp:
			mpgfp.write("P\n")
			mpgfp.flush()	# needed for immediate control
			# print("pausing")
			time.sleep(1)
			if GPIO.input(40) == 0: # still pressed - terminate
				print("Stopping rd")
				os.system('killall -s 9 rd')
				print("Stopping mpg123")
				os.system('killall -s 9 mpg123')
				# proc.terminate() doesn't always kill the process
				print("Exiting")
				quit()
			continue

	if GPIO.input(38) == 0: # switch is active LOW, Normally HIGH
		with open("/tmp/mpgout","a") as rdfp:
			rdfp.write("REW\n")
			rdfp.flush()
			# print("rewinding")
			time.sleep(1)
			continue
	if GPIO.input(36) == 0: # switch is active LOW, Normally HIGH
		with open("/tmp/mpgout","a") as rdfp:
			rdfp.write("NEXT\n")
			rdfp.flush()
			# print("next")
			time.sleep(1)
			continue
	time.sleep(0.1)   # keep cpu load down

