/* GtkDatabox - An extension to the gtk+ library
 * Copyright (C) 1998 - 2005  Dr. Roland Bock
 * 2017 - h2d -- h2d-teseo at programmer dot net (ported the plugin from gtkdatabox 0.4.0.1 to version >= 0.9.1.1).
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <stdio.h>

#include <gtk/gtk.h>
#include <gtkdatabox.h>
#include <gtkdatabox_points.h>
#include <gtkdatabox_lines.h>
#include <gtkdatabox_bars.h>
#include <gtkdatabox_markers.h>
#include <gtkdatabox_cross_simple.h>
#include <math.h>

#define POINTS 2000

extern GtkSpinButton * teseo_spbtn_arm_shift;

/*----------------------------------------------------------------
 *  databox signals
 *----------------------------------------------------------------*/

const gchar *
get_name_of_current_signal (gpointer instance)
{
   GSignalInvocationHint *ihint;

   ihint = g_signal_get_invocation_hint (instance);

   return g_signal_name (ihint->signal_id);
}

static gint
handle_signal_zoomed (GtkDatabox * box)
{
   gfloat left, right, top, bottom;

   gtk_databox_get_visible_limits (box, &left, &right, &top, &bottom);
   printf ("Name of the signal: %s\n", get_name_of_current_signal (box));
   printf ("It tells you that the GtkDatabox has zoomed to the following\n");
   printf
      ("rectangle (data coordinates only, pixels don't make sense here):\n");
   printf ("top_left (X,Y)=(%g, %g), bottom_right (X,Y)=(%g, %g)\n",
           left, top, right, bottom);

   return 0;
}

static gint
handle_signal_selection_stopped (GtkDatabox * box,
                                 GtkDataboxValueRectangle * selectionValues
                                 /*, void *unused */ )
{
   printf ("Name of the signal: %s\n", get_name_of_current_signal (box));
   printf ("It tells you that the user has stopped changing the selection\n");
   printf ("box, i.e. the mouse button is released now.\n");
   printf ("Data: corner1 (X,Y)=(%g, %g), corner2 (X,Y)=(%g, %g)\n",
           selectionValues->x1, selectionValues->y1, selectionValues->x2,
           selectionValues->y2);

   return 0;
}

static gint
handle_signal_clicked (GtkDatabox * box, GtkWidget *entry)
{

   printf ("The button was clicked\n");
   //printf ("Get the values if want %s\n" ,gtk_entry_get_text(GTK_ENTRY(entry)));

   //gchar NUMSTRING[20];
   //sprintf(NUMSTRING,"%s",gtk_entry_get_text(GTK_ENTRY(entry)));
   gchar * NUMSTRING= g_strdup (gtk_entry_get_text(GTK_ENTRY(entry)));
   gdouble num = g_strtod(NUMSTRING,NULL);
   printf ("Get the values if want %s = %f\n", NUMSTRING, num );

   gtk_spin_button_set_value (teseo_spbtn_arm_shift,num );

   return 0;
}


static gint
handle_signal_selection_canceled (GtkDatabox * box /*, void *unused*/)
{
   printf ("Name of the signal: %s\n", get_name_of_current_signal (box));
   printf ("It tells you that the user has dismissed the selection box\n");

   return 0;
}

enum {
  SHOW_BOX,
  SHOW_ACTUAL_X,
  SHOW_ACTUAL_Y,
  SHOW_MARKED_X,
  SHOW_MARKED_Y,
  SHOW_DELTA_X,
  SHOW_DELTA_Y,
  SHOW_NUM_ENTRIES
};


static GtkWidget *
show_entry (GtkWidget *hbox, gchar *text)
{
  GtkWidget *frame;
  GtkWidget *entry;

  frame = gtk_frame_new (text);
  gtk_container_add (GTK_CONTAINER(hbox), frame);
  entry = gtk_entry_new ();
  gtk_widget_set_size_request (entry, 20, -1);
  gtk_editable_set_editable (GTK_EDITABLE(entry), FALSE);
  gtk_container_add (GTK_CONTAINER(frame), entry);

  return entry;
}

static gint
show_motion_notify_cb (GtkWidget ** entries, GdkEventMotion * event
                       /*, GtkWidget *widget */ )
{
   gfloat x, y;
   gchar *text;
   GtkDatabox *box = GTK_DATABOX (entries[SHOW_BOX]);

   x = gtk_databox_pixel_to_value_x (box, event->x);
   y = gtk_databox_pixel_to_value_y (box, event->y);

   text = g_strdup_printf ("%g", x);
   gtk_entry_set_text (GTK_ENTRY (entries[SHOW_ACTUAL_X]), text);
   g_free ((gpointer) text);
   text = g_strdup_printf ("%g", y);
   gtk_entry_set_text (GTK_ENTRY (entries[SHOW_ACTUAL_Y]), text);
   g_free ((gpointer) text);

   return FALSE;
}

static gint
show_marked_cb (GtkDatabox * box, GdkEventButton * event,
                      GtkWidget ** entries)
{
   gfloat x, y;
   gchar *text;

   if (!(event->button == 1 || event->button == 2))
      return FALSE;

   x = gtk_databox_pixel_to_value_x (box, event->x);
   y = gtk_databox_pixel_to_value_y (box, event->y);

   text = g_strdup_printf ("%g", x);
   gtk_entry_set_text (GTK_ENTRY (entries[SHOW_MARKED_X]), text);
   g_free ((gpointer) text);
   text = g_strdup_printf ("%g", y);
   gtk_entry_set_text (GTK_ENTRY (entries[SHOW_MARKED_Y]), text);
   g_free ((gpointer) text);

   return FALSE;
}

static void
show_changed_cb (GtkDatabox * box,
                 GtkDataboxValueRectangle * selectionValues,
                 GtkWidget ** entries)
{
   gchar *text;

   text = g_strdup_printf ("%g", selectionValues->x2 - selectionValues->x1);
   gtk_entry_set_text (GTK_ENTRY (entries[SHOW_DELTA_X]), text);
   g_free ((gpointer) text);
   text = g_strdup_printf ("%g", selectionValues->y2 - selectionValues->y1);
   gtk_entry_set_text (GTK_ENTRY (entries[SHOW_DELTA_Y]), text);
   g_free ((gpointer) text);

   text = g_strdup_printf ("%g", selectionValues->x2);
   gtk_entry_set_text (GTK_ENTRY (entries[SHOW_ACTUAL_X]), text);
   g_free ((gpointer) text);
   text = g_strdup_printf ("%g", selectionValues->y2);
   gtk_entry_set_text (GTK_ENTRY (entries[SHOW_ACTUAL_Y]), text);
   g_free ((gpointer) text);
}

void
setT (GtkDataboxGraph * graph,
      guint index,
      GtkDataboxMarkersTextPosition label_position, gchar * label, gboolean boxed)
{
   GtkDataboxMarkers *markers = GTK_DATABOX_MARKERS (graph);

   g_return_if_fail (GTK_DATABOX_IS_MARKERS (markers));

   gtk_databox_markers_set_label (markers, index, label_position, label, boxed);
}



 GtkWidget* teseo_create_databox (
		gint num_points, gfloat *X, gfloat *Y,
		GdkColor color, guint type, guint size,
		const gchar *title, const gchar *description,
		const gchar * labelX , const gchar *labelY,
		gboolean showbutton)
{
   GtkWidget *window = NULL;
   GtkWidget *box1;
   GtkWidget *box2;
   GtkWidget *close_button;
   GtkWidget *box, *table;
   GtkWidget *crosses, *markers;
   GtkWidget *label;
   GtkWidget *separator;
   gint index;
   GtkWidget **entries;
   GtkWidget *hbox;


   GtkDataboxGraph *graph, *grid;
   gfloat *Xlab;
   gfloat *Ylab;

   GdkColor bg_color;
   GdkColor grid_color;
   GdkColor axis_color;

   gdk_color_parse("white", &bg_color);
   gdk_color_parse("light blue", &grid_color);
   gdk_color_parse("black", &axis_color);



   entries=g_new0(GtkWidget *, SHOW_NUM_ENTRIES);

   window = gtk_window_new (GTK_WINDOW_TOPLEVEL);


   g_signal_connect (GTK_OBJECT (window), "destroy",
		     G_CALLBACK (gtk_true), NULL);

   // gtk_window_set_title (GTK_WINDOW (window), "GtkDatabox: Signals Examples");
   gtk_window_set_title (GTK_WINDOW (window), (title == NULL)? "" : title);

   gtk_window_set_modal(GTK_WINDOW(window),TRUE);
   gtk_window_set_default_size(GTK_WINDOW(window),500,500);

   gtk_container_set_border_width (GTK_CONTAINER (window), 0);

   box1 = gtk_vbox_new (FALSE, 0);
   gtk_container_add (GTK_CONTAINER (window), box1);

   label =
      gtk_label_new((description == NULL)? "" : description);

   gtk_box_pack_start (GTK_BOX (box1), label, FALSE, FALSE, 0);
   separator = gtk_hseparator_new ();
   gtk_box_pack_start (GTK_BOX (box1), separator, FALSE, FALSE, 0);

   hbox=gtk_hbox_new(TRUE,3);
   gtk_box_pack_start(GTK_BOX(box1), hbox, FALSE, TRUE, 0);

   entries[SHOW_ACTUAL_X] = show_entry (hbox, "Actual X");
   entries[SHOW_ACTUAL_Y] = show_entry (hbox, "Actual Y");
   entries[SHOW_MARKED_X] = show_entry (hbox, "Marked X");
   entries[SHOW_MARKED_Y] = show_entry (hbox, "Marked Y");
   entries[SHOW_DELTA_X]  = show_entry (hbox, "Delta X");
   entries[SHOW_DELTA_Y]  = show_entry (hbox, "Delta Y");

   gtk_databox_create_box_with_scrollbars_and_rulers (&box, &table,
						      TRUE, TRUE, TRUE, TRUE);
   entries[SHOW_BOX]  = box;

   /*setting the background color*/
   gtk_widget_modify_bg (box, GTK_STATE_NORMAL, &bg_color);

   // points for a graph, whose type depends on what's requested: histogram or curve
   if(type == GTK_DATABOX_TYPE_BARS)
	   graph = gtk_databox_bars_new(num_points,X,Y,&color,size);
   else //if(type == GTK_DATABOX_TYPE_LINES)
	   graph = gtk_databox_lines_new(num_points,X,Y,&color,size);
   gtk_databox_graph_add(GTK_DATABOX(box),graph);



   /*setting the grid*/


 //  grid = gtk_databox_grid_new (7, 7, &grid_color, 2); // h2d tests:
   	   	   	   	   	   	   	   	   	   	   	   	   	   	 // produces a segfault systematically on mac osx (gtkdtbx 0.9.1.1)
   	   	   	   	   	   	   	   	   	   	   	   	   	   	 // and sometimes on linux (gtkdtbx 0.9.2.0)
   	   	   	   	   	   	   	   	   	   	   	   	   	   	 // TODO: something to fix here...
   // so use a cross rather than a grid
   grid = gtk_databox_cross_simple_new(&axis_color,1);
   gtk_databox_graph_add (GTK_DATABOX (box), grid);


 /*setting the axis labels*/

   Xlab = g_new0 (gfloat, 2);
   Ylab = g_new0 (gfloat, 2);

   /*Position of labels*/
   Xlab [0] = +10.; Ylab [0] = - 5.;
   Xlab [1] = -12.; Ylab [1] = + 25.;


   markers = gtk_databox_markers_new (2, Xlab, Ylab, &color, 2,
				   GTK_DATABOX_MARKERS_NONE);
   gtk_databox_graph_add (GTK_DATABOX (box), markers);
   setT(markers,0,GTK_DATABOX_MARKERS_TEXT_E, labelX, FALSE);
   setT(markers,1,GTK_DATABOX_MARKERS_TEXT_N, labelY, FALSE);



   gtk_databox_auto_rescale(GTK_DATABOX(box), 0.05);

   gtk_box_pack_start (GTK_BOX (box1), table, TRUE, TRUE, 0);

   separator = gtk_hseparator_new ();
   gtk_box_pack_start (GTK_BOX (box1), separator, FALSE, TRUE, 0);

   box2 = gtk_vbox_new (FALSE, 10);
   gtk_container_set_border_width (GTK_CONTAINER (box2), 10);
   gtk_box_pack_end (GTK_BOX (box1), box2, FALSE, TRUE, 0);

   if(showbutton){
	close_button = gtk_button_new_with_label ("Choose");
	g_signal_connect (GTK_OBJECT (close_button), "clicked",
				G_CALLBACK (handle_signal_clicked),
				GTK_OBJECT (   entries[SHOW_MARKED_X] ));


	gtk_box_pack_start (GTK_BOX (box2), close_button, TRUE, TRUE, 0);
	GTK_WIDGET_SET_FLAGS (close_button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default (close_button);
   }
   g_signal_connect (GTK_OBJECT (box), "zoomed",
		     G_CALLBACK (handle_signal_zoomed),
                     NULL);
   g_signal_connect (GTK_OBJECT (box), "selection-finalized",
		     G_CALLBACK (handle_signal_selection_stopped),
                     NULL);
   g_signal_connect (GTK_OBJECT (box), "selection-canceled",
		     G_CALLBACK (handle_signal_selection_canceled),
		     NULL);
   g_signal_connect_swapped (GTK_OBJECT (GTK_DATABOX (box)),
                     "motion_notify_event",
                     G_CALLBACK (show_motion_notify_cb),
                     entries);
   g_signal_connect (GTK_OBJECT (box), "button_press_event",
                     G_CALLBACK (show_marked_cb),
                     entries);
   g_signal_connect (GTK_OBJECT (box), "selection-started",
                     G_CALLBACK (show_marked_cb),
                     entries);
   g_signal_connect (GTK_OBJECT (box), "selection-changed",
                            G_CALLBACK (show_changed_cb),
                            entries);
  return window;
   //gtk_widget_show_all (window);
}

