This repository consists of two programs:
rd.c and rd_control.py

These two programs control the mpg123 music player software and allow 
remote control via some switches attached to a raspberry pi zero.

this can work on any computer with simular hardware (3 momentary switches).

rd_control.py is a python (version 2.7) program that starts rd and mpg123
running. It reads the status of 3 momentary switches and controls playback
of mpg123 thru it's remote capability. 

rd_control.py can be started from /etc/rc.d at bootup. It will start
rd and mpg123 running is a paused state. A short press of the S1 switch
will start playback. A short press of S1 will toggle between play and pause.

A long press of S1 will stop playback and terminate all 3 programs (rd,
mpg123 and rd_control.py).

A short press on S2 will rewind the current song to the beginning.
A short press on S3 will start playing the next song in the playlist.

When started, rd looks for a file called playlist.fav in the current directory.
This is defined in the source (rd.c) and can be changed and recompiled if
needed.



rd.c is a simple program to remotely control the mpg123 mp3 player. 

It will create 2 fifo's /tmp/mpgfifo and /tmp/mpgout
rd now waits for input from mpg123. This is from the comments in rd.c:

	Commands this program responds to are:
	PREV	Play the song before the current
	REW	Rewind the current song to the beginning
	NEXT	Play the next song

	The above are sent here by:
		echo NEXT|PREV|REW >> /tmp/mpgout

	And from the mpg123 program:
	@P 0	Plays the next song in the list

	The above is generated when:
		echo STOP >> /tmp/mpgfifo
	is sent to the mpg123 program

	When this program starts it reads a file called 'playlist.fav'
	and creates a random sorted list of the songs. It then plays them
	through until the end when it starts over at the beginnig.

	the playlist is created from:
	find /home/kurt/mp3 -name "*.mp3" > playlist.fav

	The playlist file can be edited by hand to pull un-wanted songs
	Also multiple playlists can be created (xmas, love songs, etc)


There should be enough comments to make sense of how things should run.

- kurt


