rd.c is a simple program to remotely control the mpg123 mp3 player. 

As of 21 March 2018 it is working but not complete. rd is meant 
to run on a raspberry pi 0 with some control buttons, but there is
no reason it can't run on any box that mpg123 runs on.

Using the program:
start it: 

./rd

It will create 2 fifo's in /tmp: mpgfifo and mpgout
Then start mpg123: 

mpg123 --fifo /tmp/mpgfifo -R abcd >> /tmp/mpgout

rd now waits for input from mpg123. This is from the comments in rd.c:

*	Commands this program responds to are:
*	PREV	Play the song before the current
*	REW	Rewind the current song to the beginning
*	NEXT	Play the next song
*
*	The above are sent here by:
*		echo NEXT|PREV|REW >> /tmp/mpgout
*
*	And from the mpg123 program:
*	@P 0	Plays the next song in the list
*
*	The above is generated when:
*		echo STOP >> /tmp/mpgfifo
*	is sent to the mpg123 program
*
*	When this program starts it reads a file called 'playlist.fav'
*	and creates a random sorted list of the songs. It then plays them
* 	through until the end when it starts over at the beginnig.
*
*	the playlist is created from:
*	find /home/kurt/mp3 -name "*.mp3" > playlist.fav
*
*	The playlist file can be edited by hand to pull un-wanted songs
*	Also multiple playlists can be created (xmas, love songs, etc)


