/**
There is absolutely no gaurantee on the proper working of this program. 
This was written in under two days & uses a lot of Hacks to get around
problems that were faced in using libgtk and ffmpeg.

Users are requested to refer the following 

GTK Tutorials         - http://library.gnome.org/devel/gtk-tutorial/2.21/
FFmpeg x264 mappings  - http://sites.google.com/site/linuxencoding/x264-ffmpeg-mapping 
x264 Option           - http://mewiki.project357.com/wiki/X264_Settings

Author: Rohit Nair, roguehit@gmail.com
**/


#include "common.h"
#include "gui.h"

gint delete_event( GtkWidget *widget,
                   GdkEvent  *event,
                   gpointer   data )
{
    gtk_main_quit();
    return(FALSE);
}

char * strrev(char * string) {
  int length = strlen(string);
  char * result = malloc(length+1);
  if( result != NULL ) {
    int i,j;                                         result[length] = '\0';
    for ( i = length-1, j=0;   i >= 0;   i--, j++ )  result[j] = string[i];
  }
  return result;
}


void store_filename(GtkFileSelection *file_selection, gpointer data)
{

  int i = 0;
  int j = 0;
  int start = 0, end = 0;
   
  filename = (gchar*) gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_box));
  gtk_entry_set_text (GTK_ENTRY (entry1), filename);

  
  transcode.filepath=(char*)malloc(strlen(filename)+1);
  strcpy(transcode.filepath,filename);
  transcode.filename=(char*)malloc(100);
  
  for(i = strlen(filename) ; filename[i] != '/' ; i-- ){
  if(filename[i] == '.')
  end = i;
  }
  start = ++i;
  for(i = start,j = 0 ; i < end ; i++,j++)
  transcode.filename[j] = filename [i];
  
  transcode.filename[j] = '\0';
  
  //fprintf(stderr,"Start:%d End:%d Filename %s\n",start,end,transcode.filename);  
  return;
}

void source_select( GtkWidget *widget,
               gpointer   *entry )
{
    
  file_selection_box = gtk_file_selection_new("Please select a file for editing.");

  /*-- Link the ok button to the store_filename function --*/   
  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(file_selection_box)->ok_button), 
                                  "clicked", GTK_SIGNAL_FUNC (store_filename), NULL);
                             
  /*-- Destroy the dialog box when the user clicks the ok_button --*/
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selection_box)->ok_button), 
            "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer) file_selection_box);

  /*-- Destroy the dialog box when the user clicks the cancel_button --*/
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(file_selection_box)->cancel_button), 
                "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer) file_selection_box);
     
  /* Display the file dialog box */
  gtk_widget_show (file_selection_box);

}

void* transcode_video(void* temp)
{
ProgressData* progress =(ProgressData*) temp;

char ff_option[2000]; 	
print_selected_option();
fprintf(stderr,"Thread Created\n");
struct timeval now;
/*This does not work from inside of a thread*/
//show_popup("Transcoder","Done!");
gettimeofday(&now,NULL);
progress->call_next= 1;

gtk_button_set_label (GTK_BUTTON (progress->button),"Transcoding in Progress.....");
gtk_widget_show(progress->button);
gtk_progress_bar_set_text (GTK_PROGRESS_BAR (progress->progressbar), "Working ....");   

/*
Invoke FFmpeg here
FIXME system() sucks but no other option
FIXME please read ffmpeg.c & try making another module
*/
gtk_widget_show(progress->button);
sprintf(ff_option,"ffmpeg -y -i %s -vcodec libx264 -vpre slow -vpre main -cqp 28 -sc_threshold 60 -i_qfactor 1 -b_qfactor 1 -loglevel quiet -s %dx%d %s/%s.%s 2> frame", transcode.filepath,transcode.x_res,transcode.y_res,transcode.dest_path,transcode.filename,transcode.format);
//printf("%s\n",ff_option);
g_timeout_add (1000, progress_timeout, progress);

system(ff_option);

gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress->progressbar), 0.0);
transcode.job_running = 0;
progress->call_next = 0;
gtk_button_set_label (GTK_BUTTON (progress->button),"Start Transcoding");
gtk_progress_bar_set_text (GTK_PROGRESS_BAR (progress->progressbar), "Done !");   

gtk_widget_show(progress->button);
system("rm frame");
pthread_exit(0);
}


void start_ffmpeg (GtkWidget *widget, gpointer data )
{
pthread_t ffmpeg_invoker;
int ret;
ProgressData *progress = (ProgressData*) data;



if(!option_sanity_check())
{
printf("Problem in GUI Selections\n");
return;
}

/*
Check to see if Default Resolution is required
*/

if(transcode.x_res==0)
if( (ret = get_resolution(transcode.filepath)) == 0 )
{
printf("Error in Setting Resolution %d\n",ret);
return;
}


if(!transcode.job_running)
{
transcode.job_running=1;
printf("Time to start ffmpeg on a thread\n");
pthread_create(&ffmpeg_invoker,NULL,transcode_video,(void*) progress);
pthread_detach(ffmpeg_invoker);
}

else
fprintf(stderr,"One job is already running\n");
return;
}


void change_in_entry1( GtkWidget *widget,
               gpointer   *entry )
{
   gchar *entry_text;
   entry_text = (gchar*) gtk_entry_get_text(GTK_ENTRY(entry));
   printf("Entry contents: %s\n", entry_text);
}

void change_in_entry2( GtkWidget *widget,
               gpointer *entry )
{
  
   gchar *entry_text;
   errno=0;
   entry_text = (gchar*) gtk_entry_get_text(GTK_ENTRY(entry));
   printf("Creating directory%s\n", entry_text);
   char error[200];	
   char* pathname =(char*)malloc(sizeof(char)*strlen(entry_text)+1);
   
   if(strcpy(pathname,entry_text)==NULL)
	fprintf(stderr,"Error Copying the string\n");

   if (mkdir(pathname, S_IRUSR | S_IWUSR | S_IXUSR ))
   	{
	fprintf(stderr,"Error creating the directory, Errno %d\n",errno);
	sprintf(error,"Error creating the directory, Errno %d\n",errno);
	strcpy(transcode.dest_path,pathname);
        }

   else {
   	fprintf(stderr,"Directory created sucessfully\n");
	transcode.dest_path=(char*)malloc(strlen(pathname)+1);
	strcpy(transcode.dest_path,pathname);   
 	}
   free(pathname);
   return;
}

void resolution_call_back( GtkButton *b, gpointer rb )
{
    radio *temp=(radio*)rb;
    
    
    if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( temp->rb ) ) ) 
    fprintf(stderr,"Data is %d\n",temp->res);	
    
    switch(temp->res){
	case 480: transcode.x_res=640;
		  transcode.y_res=480;
	break;

	case 720: transcode.x_res=1280;
		  transcode.y_res=720;
	break;

	case 1080: transcode.x_res=1920;
		   transcode.y_res=1080;
	break;

	case 0: transcode.x_res=0;
	break;
}
	

}
void format_call_back( GtkButton *b, gpointer rb )
{
    radio *temp=(radio*)rb;
    transcode.format=(char*)malloc(strlen(temp->format)+1);
    strcpy(transcode.format,temp->format);

    if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( temp->rb ) ) ) 
    printf("Format is %s\n",temp->format);	
}

static gboolean progress_timeout( gpointer data )
{
  ProgressData * progress = (ProgressData *)data;
  FILE *f1;
  int c;
  int temp[5]; 
  int i = 0;
  float time;
  float new_val;   
  
  if(!progress->call_next)
  return FALSE;  

  f1 = fopen("frame","r");
  while( (c = getc(f1)) != EOF )
  {
  if(c == 0x20)
  {i=0;continue;}

  temp[i++] = c;
  i %= 5;
  if(temp[0]=='t' && temp[1]=='i' && temp[2]=='m' && temp[3]=='e' && temp[4]=='=')
  fscanf(f1,"%f",&time);
  }

  if(time > transcode.duration) 
  time = 0;

  new_val = time / transcode.duration;
  //new_val = gtk_progress_bar_get_fraction (GTK_PROGRESS_BAR (progress->progressbar)) + 0.01; 
  
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress->progressbar), new_val);
  fclose(f1);

  return TRUE ; 
 
}

int main( int argc, char *argv[] )
{
    /* GtkWidget is the storage type for widgets */
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *mainbox,*box2,*box3;
    GtkWidget *label;
    GtkWidget *frame;
    fclose(stdout);
    ProgressData *progress;
    /*FIXME Use array of pointers*/
    radio *a,*b,*c,*d,*e,*f,*g,*h;

    /*FIXME wtf, Please*/
    a=(radio*)malloc(sizeof(radio));
    b=(radio*)malloc(sizeof(radio));
    c=(radio*)malloc(sizeof(radio));
    d=(radio*)malloc(sizeof(radio));
    e=(radio*)malloc(sizeof(radio));
    f=(radio*)malloc(sizeof(radio));
    g=(radio*)malloc(sizeof(radio));
    h=(radio*)malloc(sizeof(radio));
 
    char home[250];

    /* This is called in all GTK applications. Arguments are parsed
     * from the command line and are returned to the application. */
    gtk_init (&argc, &argv);

    
    window = ( GtkWindow* )g_object_new( GTK_TYPE_WINDOW,
                                      "title", "Transcoder 1.0",
                                      "default-height", 150,
                                      "default-width", 400,
                                      "border-width", 10, NULL );

    gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                        GTK_SIGNAL_FUNC (delete_event), NULL);

     /* We create a "mainbox" to pack widgets into.*/

    mainbox = gtk_vbox_new(FALSE, 0);
    
    box2 = gtk_hbox_new(FALSE, 0);

    box3 = gtk_hbox_new(FALSE, 0);
    /* Put the box into the main window. */
    gtk_container_add (GTK_CONTAINER (window), mainbox); 

    label = gtk_label_new ("____________________________________________\n\nSource Video Selection");
    gtk_box_pack_start (GTK_BOX (mainbox), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    entry1 = gtk_entry_new_with_max_length (500);
    /* Creates a new button with the label "Button". */
    button = gtk_button_new_with_label ("Select Source Video ");
    
    /* Now when the button is clicked, we call the "callback" function
     * with a pointer to "button" as its argument */

    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (source_select), entry1);
    
    /* Instead of gtk_container_add, we pack this button into the invisible
     * box, which has been packed into the window. */
    gtk_box_pack_start(GTK_BOX(box2), button, TRUE, TRUE, 0);
    
    /* Always remember this step, this tells GTK that our preparation for
     * this button is complete, and it can now be displayed. */
    gtk_widget_show(button);
        
    gtk_signal_connect(GTK_OBJECT(entry1), "activate",
                       GTK_SIGNAL_FUNC(change_in_entry1),
                       entry1);

    gtk_entry_set_text (GTK_ENTRY (entry1), "<Source File>");
    gtk_entry_select_region (GTK_ENTRY (entry1),0, GTK_ENTRY(entry1)->text_length);
    gtk_box_pack_start (GTK_BOX (box2), entry1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(mainbox), box2, TRUE, TRUE, 0);

    gtk_widget_show (entry1);
    gtk_widget_show(box2);



     label = gtk_label_new ("____________________________________________\n\nEnter Destination Folder");
  //gtk_container_add (GTK_CONTAINER (frame), label);
     gtk_box_pack_start (GTK_BOX (mainbox), label, FALSE, FALSE, 0);
     gtk_widget_show (label);

 
    entry2 = gtk_entry_new_with_max_length (500);
    gtk_signal_connect(GTK_OBJECT(entry2), "activate",
                       GTK_SIGNAL_FUNC(change_in_entry2),
                       entry2);

   

    sprintf(home,"%s",getenv("PWD"));
    transcode.dest_path=(char*)malloc(strlen(home)+1);
    strcpy(transcode.dest_path,home);
    gtk_entry_set_text (GTK_ENTRY (entry2),home);
    gtk_entry_select_region (GTK_ENTRY (entry2),0, GTK_ENTRY(entry2)->text_length);
    gtk_box_pack_start (GTK_BOX (box3), entry2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(mainbox), box3, TRUE, TRUE, 0); 
    gtk_widget_show (entry2);
    gtk_widget_show(box3);


    label = gtk_label_new ("____________________________________________\n");
  //gtk_container_add (GTK_CONTAINER (frame), label);
     gtk_box_pack_start (GTK_BOX (mainbox), label, FALSE, FALSE, 0);
     gtk_widget_show (label);

     GtkWidget *ver_box= gtk_vbox_new( FALSE, 0 );
     GtkWidget *hor_box= gtk_hbox_new( FALSE, 50 );    



#if 1 
 

   frame = gtk_frame_new ("Select Output Resolution");
   gtk_box_pack_start( GTK_BOX( hor_box ),ver_box , FALSE, FALSE, 0); 
   gtk_container_add (GTK_CONTAINER (ver_box), frame);
   
   transcode.x_res=640;
   transcode.y_res=480;
   GtkWidget *radio_button_1= gtk_radio_button_new_with_label( NULL, "480p" );
   a->res=480;
   a->rb=radio_button_1;  
   g_signal_connect( G_OBJECT( radio_button_1 ),"clicked",G_CALLBACK( resolution_call_back ),(gpointer)a );  
   gtk_box_pack_start( GTK_BOX( ver_box ),radio_button_1 , FALSE, FALSE, 0); 
  
   


   GtkWidget *radio_button_2= gtk_radio_button_new_with_label_from_widget(  GTK_RADIO_BUTTON( radio_button_1 ), "720p" );
   b->res=720;
   b->rb=radio_button_2;  
   g_signal_connect( G_OBJECT( radio_button_2 ),"clicked",G_CALLBACK( resolution_call_back ),(gpointer)b );
   gtk_box_pack_start( GTK_BOX( ver_box ),radio_button_2, FALSE, FALSE, 0); 



   GtkWidget *radio_button_3= gtk_radio_button_new_with_label_from_widget(  GTK_RADIO_BUTTON( radio_button_2 ), "1080p" );
   c->res=1080;
   c->rb=radio_button_3;  
   g_signal_connect( G_OBJECT( radio_button_3 ),"clicked",G_CALLBACK( resolution_call_back ),(gpointer)c );
   gtk_box_pack_start( GTK_BOX( ver_box ),radio_button_3 , FALSE, FALSE, 0); 
 


   GtkWidget *radio_button_4= gtk_radio_button_new_with_label_from_widget(  GTK_RADIO_BUTTON( radio_button_3 ), "Original Resolution" );
   d->res=0;
   d->rb=radio_button_4;  
   g_signal_connect( G_OBJECT( radio_button_4 ),"clicked",G_CALLBACK( resolution_call_back ),(gpointer)d );
   gtk_box_pack_start( GTK_BOX( ver_box ),radio_button_4 , FALSE, FALSE, 0); 
  

  // gtk_container_add( GTK_CONTAINER( window ), mainbox );
   
  
#endif    
    
    ver_box= gtk_vbox_new( FALSE, 0 );

   frame = gtk_frame_new ("Select Output Format");
   gtk_box_pack_start( GTK_BOX( hor_box ),ver_box , FALSE, FALSE, 0); 
   gtk_container_add (GTK_CONTAINER (ver_box), frame);


    
#if 1
   transcode.format=(char*)malloc(4);
   strcpy(transcode.format,"mp4");
   GtkWidget *radio_button_11= gtk_radio_button_new_with_label( NULL, "MP4" );
   strcpy(e->format,"mp4");  
   e->rb=radio_button_11; 
   g_signal_connect( G_OBJECT( radio_button_11 ),"clicked",G_CALLBACK( format_call_back ),(gpointer)e );

   gtk_box_pack_start( GTK_BOX( ver_box ),radio_button_11, FALSE, FALSE, 0); 
  



   GtkWidget *radio_button_22= gtk_radio_button_new_with_label_from_widget( GTK_RADIO_BUTTON( radio_button_11 ), "AVI" );
   strcpy(f->format,"avi");   
   f->rb=radio_button_22; 
   g_signal_connect( G_OBJECT( radio_button_22 ),"clicked",G_CALLBACK( format_call_back ),(gpointer)f );
   
   gtk_box_pack_start( GTK_BOX( ver_box ),radio_button_22 , FALSE, FALSE, 0); 
  



  GtkWidget *radio_button_33=  gtk_radio_button_new_with_label_from_widget(  GTK_RADIO_BUTTON( radio_button_22 ), "MPEG2 TS" );
   strcpy(g->format,"ts");   
   g->rb=radio_button_33; 
   g_signal_connect( G_OBJECT( radio_button_33 ),"clicked",G_CALLBACK( format_call_back ),(gpointer)g );

   gtk_box_pack_start( GTK_BOX( ver_box ),radio_button_33 , FALSE, FALSE, 0); 


 GtkWidget *radio_button_44=  gtk_radio_button_new_with_label_from_widget(  GTK_RADIO_BUTTON( radio_button_33 ), "FLV" );
   strcpy(h->format,"flv");   
   h->rb=radio_button_44; 
   g_signal_connect( G_OBJECT( radio_button_44 ),"clicked",G_CALLBACK( format_call_back ),(gpointer)h );
   
   gtk_box_pack_start( GTK_BOX( ver_box ),radio_button_44 , FALSE, FALSE, 0); 
gtk_box_pack_start( GTK_BOX( mainbox ),hor_box , FALSE, FALSE, 0); 


#endif

    label = gtk_label_new ("____________________________________________");
  //gtk_container_add (GTK_CONTAINER (frame), label);
    gtk_box_pack_start (GTK_BOX (mainbox), label, FALSE, FALSE, 0);
    gtk_widget_show (label);
    /* The order in which we show the buttons is not really important, but I
     * recommend showing the window last, so it all pops up at once. */
    progress = g_malloc (sizeof (ProgressData));

    progress->button  = gtk_button_new_with_label ("Start Transcoding");
    
    /* Now when the button is clicked, we call the "callback" function
     * with a pointer to "button 1" as its argument */
    
    gtk_signal_connect (GTK_OBJECT (progress->button), "clicked", GTK_SIGNAL_FUNC (start_ffmpeg), (gpointer) progress);

    gtk_box_pack_start(GTK_BOX(mainbox), progress->button, TRUE, TRUE, 0);
   
    progress->progressbar = gtk_progress_bar_new ();

    label = gtk_label_new ("____________________________________________");
  //gtk_container_add (GTK_CONTAINER (frame), label);
    gtk_box_pack_start (GTK_BOX (mainbox), label, FALSE, FALSE, 0);
    gtk_widget_show (label);    

 
    //gtk_progress_bar_set_text (GTK_PROGRESS_BAR (progress->progressbar), "Progress Bar");   
   
    gtk_box_pack_start(GTK_BOX(mainbox), progress->progressbar, TRUE, TRUE, 0);   
    
   
    //gtk_widget_show(mainbox);
    gtk_widget_show_all( GTK_WIDGET( window ) );

    /* Rest in gtk_main and wait for the fun to begin! */
    gtk_main ();

    return(0);
}

