#ifndef GUIH_GUI_H
#define GUIH_GUI_H

#include "common.h"

static char *value_string(char *buf, int buf_size, double val, const char *unit)
{
    snprintf(buf, buf_size, "%f", val);
    
    return buf;
}

static char *time_value_string(char *buf, int buf_size, int64_t val, const AVRational *time_base)
{
    if (val == AV_NOPTS_VALUE) {
        snprintf(buf, buf_size, "N/A");
    } else {
        value_string(buf, buf_size, val * av_q2d(*time_base), unit_second_str);
    }

    return buf;
}

int option_sanity_check()
{
AVFormatContext *pFormatCtx;
av_register_all();
AVStream *stream ;
int videoStream=0,i=0;
 char val_str[128];
/*
Perform File based Sanity Checks before beginning the transcoding Process
*/

if(transcode.filepath==NULL)
{
fprintf(stderr,"Filename not specified\n");
show_popup("File Selection","  Please Select a Source File  ");
return 0;
}

if(av_open_input_file(&pFormatCtx, transcode.filepath, NULL, 0, NULL)!=0)
{ fprintf(stderr,"Invalid FileFormat of %s\n",transcode.filepath); show_popup("File Selection","  Invalid Source File :(  ");return 0;} 

 // Retrieve stream information
  if(av_find_stream_info(pFormatCtx)<0)
   {fprintf(stderr,"Could not find streams in the video\n");show_popup("File Selection","  Something is terribly wrong with your File  ");return 0;} 
  
  // Find the first video stream
  videoStream=-1;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1)
    {fprintf(stderr,"No Video Stream in the Source file\n");show_popup("File Selection","  Source File does not have a video stream  ");return 0;} 
  
/*
Do somemore Sanity check on the GUI Selections 
*/
stream = pFormatCtx->streams[videoStream];
transcode.duration = stream->duration;
   //fprintf(stdout,"The Duration is %d \n",atoi(time_value_string(val_str, sizeof(val_str), stream->duration,&stream->time_base)));
transcode.duration = atoi(time_value_string(val_str, sizeof(val_str), stream->duration,&stream->time_base)); 
return 1;
}



int get_resolution(char *filename)
{
  AVFormatContext *pFormatCtx;
  int             i, videoStream;
  AVCodecContext  *pCodecCtx;
  

if(filename==NULL){
fprintf(stderr,"Please Select the source file\n");
return 0;
}

  av_register_all();
  
  // Open video file
  if(av_open_input_file(&pFormatCtx, filename, NULL, 0, NULL)!=0)
   { fprintf(stderr,"Invalid FileFormat\n"); show_popup("File Selection","  Invalid Source File :(  ");return 0;} // Couldn't open file
  
  // Retrieve stream information
  if(av_find_stream_info(pFormatCtx)<0)
   {fprintf(stderr,"Could not find streams in the video\n");show_popup("File Selection","  Something is terribly wrong with your File  ");return 0;} 
  
  // Find the first video stream
  videoStream=-1;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1)
    {fprintf(stderr,"No Video Stream in the Source file\n");show_popup("File Selection","  Source File does not have a video stream  ");return 0;} 
  
   pCodecCtx=pFormatCtx->streams[videoStream]->codec;
       
   transcode.x_res=pCodecCtx->width;
   transcode.y_res=pCodecCtx->height;
   
   return 1;
}

void show_popup(char *title,char* message)
{
  GtkWidget *dialog,*label;

  dialog= gtk_dialog_new_with_buttons (title,NULL,
                                                 GTK_DIALOG_MODAL,
                                                 0,
                                                 GTK_RESPONSE_ACCEPT,
                                                 0,
                                                 GTK_RESPONSE_REJECT,
                                                 NULL);

    label = gtk_label_new (message);

    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
                        label, TRUE, TRUE, 0);
    gtk_widget_show (label);
    gtk_widget_show (dialog);

    return;
}

void print_selected_option()
{
printf("Filename : %s\nDest Path : %s\nX_Size : %d\nY_Size: %d\nFormat : %s\n",transcode.filepath,transcode.dest_path,transcode.x_res,transcode.y_res,transcode.format);

}

static gboolean progress_timeout( gpointer  );


#endif
