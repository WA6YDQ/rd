/*
*
*	rd.c - remote control for mpg123
*
*	ktheis <theis.kurt@gmail.com>
*	3/15/2018
*
*	This program controls the mpg123 music player
*	Start this program 1st (./rd) then start mpg123:
*	mpg123 --fifo /tmp/mpgfifo -R abcd >> /tmp/mpgout
*
*	Another program reads the control buttons and sends messages
*	to this program.
*
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
*
*
*	There's nothing special about compiling this: 
*	cc -o rd rd.c
*
*	It should run on any 'nix machine.
*/

#define MAXSONGS 5000	/* maximum number of songs to read in */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>

/* global variables */

/* file descripters */
int in;		
int music;
FILE *PLIST;
FILE *out;

/* playlist */
char playlist[MAXSONGS][150];	/* holds file names of songs */
int random_song_list[5000];	/* holds play order */




/**************************/
/*     GET_RESPONSE       */
/**************************/

/* get a response from the mpg123 player */
char *get_response(void) {
char c, result[150];
int ct;
char *res;

    ct = 0;
    bzero(result,sizeof(result));
    while (1) {
	if (read(in,&c,1)==0) 
	    return (NULL);
    	if (c == 0xff) 
	    return (NULL);

    	result[ct] = c;
    	if (c == '\n') {
            result[++ct] = '\0';
	    res = result;
	    return (res);
        }
	ct++;
        continue;
   }
}


/**********************/
/*       MAIN         */
/**********************/

int main(void) {
char *response;
char c;
int ct;
char cmd[130];
int songnum = 1;
int FLAG;
int song_count;
int number, i;
char *title, *artist, *album;

	/* open playlist */
	PLIST = fopen("playlist.fav","r");
	if (PLIST == NULL) {
		perror("playlist.fav");
		exit(1);
	}

	/* read in playlist */
	ct = 0;
	while(1) {
		fgets(playlist[ct++],sizeof(playlist),PLIST);
		if (feof(PLIST)) 
			break;
	}
	fclose(PLIST);
	song_count = ct-1;
	printf("Read %d Songs\n",song_count);

	/* create a random song list */
	srand48((long)(time(NULL)));
	for (ct=0; ct<song_count+1; ct++)
		random_song_list[ct]=-1;

	for (ct=0; ct<song_count+1; ct++) {
		number = (int)(drand48() * song_count + 1);
		for (i=0; i<=ct; i++)	// no duplicates
			if (random_song_list[i] == number) continue;
		random_song_list[ct] = number;
	}


	/* create fifo's */
	unlink("/tmp/mpgfifo");
	unlink("/tmp/mpgout");
	mkfifo("/tmp/mpgfifo",0666);
	mkfifo("/tmp/mpgout",0666);


	/* open fifo, wait for initial turn-on msg */
	in = open("/tmp/mpgout",O_RDONLY);	// was O_RDWR
	if (in == -1)
		perror("/tmp/mpgout");
	response = get_response();
	while (response == NULL)
		get_response();
	printf("%s",response);

	/* initialize song counter */
	ct = 0;

	/* loop */
	while (1) {
		response = get_response();

		/* no or invalid response */
		if (response == NULL)
			continue;

		/* @F shows song info and allows us to check while looping */
		if ((strncmp(response,"@F",2))==0)
		    continue;

		//printf("%s \n",response);

		/* look for external stimuli */

		/* REW - start from beginning of song */
		if ((strncmp(response,"REW",3))==0) {
			ct -= 1;
			//printf("COUNT=%d \n",ct);
			response = "@P 0\0";
		}
	
		/* PREV - song before current */
		if ((strncmp(response,"PREV",4))==0) {
			ct -= 2;
			if (ct < 0) ct = song_count;
			//printf("COUNT=%d \n",ct);
			response = "@P 0\0";
		}

		/* NEXT - song after current (the STOP command bumps the counter) */
		if ((strncmp(response,"NEXT",4))==0) {
			//printf("COUNT=%d \n",ct);
			response = "@P 0\0";
		}


		/* @S shows song info */
		if ((strncmp(response,"@S",2))==0) {
		    continue;
		}

		/* show some control messages */
		if ((strncmp(response,"@s",2))==0) {  // @silence
		    printf("%s",response);
		    continue;
		}

		/* check for information messages */
		if ((strncmp(response,"@I",2))==0) {
		    if ((strncmp(response,"@I ID3v2.title",14))==0) {
			title = index(response,':');
			if (title == NULL)
			    printf("%s",response);
			else
				printf("TITLE%s",title);
			continue;
		    }
		    if ((strncmp(response,"@I ID3v2.artist:",16))==0) {
			artist = index(response,':');
			if (artist == NULL) artist = "(unknown)";
			printf("ARTIST%s",artist);
			continue;
		    }
		    if ((strncmp(response,"@I ID3v2.album:",15))==0) {
			album = index(response,':');
			if (album == NULL) artist = "(unknown)";
			printf("ALBUM%s",album);
			continue;
		    }
		    continue;
		}

		/* playing */
		if ((strncmp(response,"@P 2",4))==0) {
		    printf("PLAYING \n");
		    continue;
		}

		/* paused */
		if ((strncmp(response,"@P 1",4))==0) {
		    printf("PAUSED \n");
		    continue;
		}

		/* stopped playing */
		if ((strncmp(response,"@P 0",4))==0) {
		    printf("STOPPED \n\n");
		    sleep(1);
		    playlist[random_song_list[ct]][strcspn(playlist[random_song_list[ct]],"\n\r")]='\0';
                    strcpy(cmd,"LOAD ");
                    strcat(cmd,playlist[random_song_list[ct]]);
                    strcat(cmd,"\n");
                    printf("File: %s\n",playlist[random_song_list[ct]]);

		    /* needs to be opened before every write */
		    music = open("/tmp/mpgfifo",O_RDWR);
		    if (music == -1)
		        perror("/tmp/mpgfifo");
		    write (music,cmd,strlen(cmd));
		
		    printf("SONG %d of %d \n",ct,song_count);
		    /* point to next song in list */
		    ct++;
		    if (ct == song_count) ct = 0; // overflow
		    continue;
		}
	
		/* unknown response */
		continue;
	}

}

