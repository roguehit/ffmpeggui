#ifndef COMMON_GUI_H
#define COMMON_GUI_H

#include <gtk/gtk.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h> 
#include <string.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>


GtkWidget *entry1,*entry2;
GtkWidget *file_selection_box;
gchar *filename;

struct _radio{
int res;
GtkWidget *rb;
char format[10];
};
typedef struct _radio radio;


/*
This structure contains all transcoding related 
members. Currently only implements resolution change 
and output formats.
*/
struct _transcode{

int x_res;	
int y_res;
float aspect;
int framerate;
char* format;
char* filename;
char* dest_path;
volatile int job_running; /* Set/Reset in threads hence volatile*/

}transcode;

/*
Helper functions used in Various Callbacks
*/

void print_selected_option();
int  option_sanity_check();
void show_popup(char*,char*);
int get_resolution(char*);

#endif
