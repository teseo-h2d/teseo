/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#ifndef _GTKADDONS_RC_H
#define _GTKADDONS_RC_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>


#define P_NAME_LENGTH 160
#define P_VALUE_LENGTH 80
#define P_MAX_NUM 400



/*!
struct mydata:w_name is the widget name, w_content_to is the line content to setup widget
*/
struct mydata{
		gchar * w_name;
		gchar * w_content_to;
} ;



struct param_struct{
	gchar* name[P_MAX_NUM];
	gchar* value[P_MAX_NUM];
	gulong current;
};


/*!
init_store_widget initialize the library token string
	\param const char * mytoken
*/
int init_store_widget(const char * mytoken);


int is_marked_name(gchar * widget_name);

/*!
is_token_widget return 1 if widget name contain token value, else it returns 0
	\param widget pointer widget
	\param token string to match
*/
int is_token_widget(GtkWidget * widget, const char * token);

/*!
is_teseo_widget return 1 if widget name contain 'teseo', else it returns 0
	\param widget pointer widget
*/
int is_teseo_widget(GtkWidget * widget);

/*!
iface_save_rc write in a file the widget values of a container
	it calls iface_save_rc_recursive
	\param file_rc file name
	\param parent_widget pointer widget where starting
*/
char iface_save_rc(const char * file_rc,  GtkWidget * parent_widget);

/*!
iface_save_rc_recursive browses recursively in a container and save in a file  the teseo widget name and its value
	it is called by iface_save_rc
	\param data current widget
	\param user_data file descript pointer. File must be open.
*/
void iface_save_rc_recursive(gpointer data, gpointer user_data);

/*!
iface_display_rc_recursive browses recursively in a container and print to stdout the teseo widget name and its value
	\param data current widget
	\param user_data string for tabulating output
*/
void iface_display_rc_recursive(gpointer data, gpointer user_data);

/*!
print_iface() test function. Browse in a cointainer and display to stdout widget name and its value
	\param data current widget
	\param user_data string for tabulating output
*/
void print_iface(gpointer data, gpointer user_data) ;

/*!
iface_load_rc read in a file the widget values of a container it calls iface_load_rc_recursive
	\param file_rc resourcefilename
	\param user_data parent widget
*/
char iface_load_rc(const char * file_rc,  GtkWidget * parent_widget );

/*!
iface_load_rc_recursive browses in a cointainer looking for a widget to setup
load the widget value.
  \param data current widget
	\param user_data  mydata structure
*/
void iface_load_rc_recursive(gpointer data, gpointer user_data);

/*!
iface_load_list read in the params the widget values of a container it calls iface_load_rc_recursive
	\param params
	\param  parent_widget
*/
char iface_load_list( struct param_struct * params,  GtkWidget * parent_widget );

/*!
list_iface write in two arraty of string the widget values of a container
	it calls iface_list_recursive
	\param parent_widget pointer widget where starting
	\param
*/
void iface_list(GtkWidget * parent_widget , struct param_struct ** params );

/*!
iface_list_recursive browses recursively in a container and save in param_name the teseo widget name and in param_value its value
	it is called by iface_list
	\param
*/

void iface_list_recursive( gpointer data, gpointer ps);

/*!
iface_list_delete destructor for param_struct
	\param
*/
void iface_list_delete(struct param_struct ** params );

/*!
iface_list_print print param_struct containt on stdout
	\param
*/
void iface_list_print(struct param_struct * params );




#endif /* _MGLADE_RC_H */
