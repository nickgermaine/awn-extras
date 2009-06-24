/*
 * Copyright (c) 2007   Rodney (moonbeam) Cryderman <rcryderman@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
 
 
#ifndef _AWN_EXTRS_UTIL
#define _AWN_EXTRS_UTIL

#include <gtk/gtk.h>
#include <glib.h>
#include <libnotify/notify.h>

/*
void notify_message
	Sends a notificication to a notification daemon.
	summary	-	message summary.
	body	-	body of message
	icon	-	full path to icon to display.  NULL indicates no icon.
	timeout	-	notificiation window timeout in ms.  0 indicates default should be used.
	returns	TRUE on success, FALSE on failure	
*/
gboolean notify_message(gchar * summary, gchar * body,gchar * icon_str,glong timeout);
/*
void notify_message_async
	Sends a notificication to a notification daemon in an asyncronous manner
	summary	-	message summary.
	body	-	body of message
	icon	-	full path to icon to display.  NULL indicates no icon.
	timeout	-	notificiation window timeout in ms.  0 indicates default should be used.
	
Implementation Notes:
	-Uses fork()
*/
void notify_message_async(gchar * summary, gchar * body,gchar * icon_str,glong timeout);
/*
void notify_message
	Sends a notificication to a notification daemon.
	summary	-	message summary.
	body	-	body of message
	icon	-	full path to icon to display.  NULL indicates no icon.
	urgency -   Urgency of notification.  
	timeout	-	notificiation window timeout in ms.  0 indicates default should be used.
	perror  -   typical g_error situation.
	returns	TRUE on success, FALSE on failure	
*/
void notify_message_extended(gchar * summary, gchar * body,gchar * icon_str,NotifyUrgency urgency,glong timeout,GError **perror);


#define SHR_KEY_CONFIG_KEY(key) key
#define SHR_KEY_FOCUS_LOSS      SHR_KEY_CONFIG_KEY("dialog_focus_loss_behavior")
#define SHR_KEY_GENERIC_PREFS   SHR_KEY_CONFIG_KEY("allow_generic_config_edit")

/* 
share_config_bool
	returns a boolean shared configuration key.
*/

gboolean share_config_bool(const gchar * key);

/*
* Create a menu item that invokes a generic applet preferences dialog.
* instance - The folder name containing the configuration key within the applets
* configuration folder.
* baseconf - If there is a default configuration location that is different than
* the instance provided.  Otherwise NULL.
* applet_name - applet name used to reference the associated schema-ini
*
*  Returns:
*    A gtk_menu_item or NULL if the generic applet preferences configuration is
*    disabled
*
*  Notes:
*    There is no need to attach the returned item to a
*  signal as this is handled by the function.
*/
GtkWidget *shared_menuitem_create_applet_prefs(gchar *instance,gchar *baseconf,
                                              gchar * applet_name);

#endif
