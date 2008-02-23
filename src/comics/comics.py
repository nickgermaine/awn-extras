#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2008 Moses Palmér
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.


import awn
import awn.extras.AWNLib
import gettext
import gobject
import gtk
import gtk.glade
import locale
import os
import sys
import tempfile

from locale import gettext as _
from os.path import join
from settings import Settings

import comics_add
import comics_manage
import comics_view

from shared import SHARE_DIR, SYS_FEEDS_DIR, USER_FEEDS_DIR, GLADE_DIR, \
	PIXMAPS_DIR, LOCALE_DIR, STRIPS_DIR, feeds

GLADE_FILE = join(GLADE_DIR, 'main.glade')


class ComicApplet(awn.AppletSimple):
	def set_visibility(self, visible):
		"""Show or hide the comic windows."""
		self.visible = visible
		
		for window in self.windows:
			window.set_visibility(visible)
	
	def create_window(self, filename = None, template = None):
		"""Creates a new strip and stores its configuration in the directory
		path."""
		if filename is None:
			f, filename = tempfile.mkstemp('.strip', '', STRIPS_DIR, True)
			os.close(f)
		settings = Settings(filename)
		if template:
			settings.update(template)
		settings['cache-file'] = filename.rsplit('.', 1)[0] + '.cache'
		window = comics_view.ComicsViewer(settings, self.visible)
		self.windows.append(window)
		window.connect('removed', self.on_window_removed)
		window.connect('updated', self.on_window_updated)
		settings.save()
	
	def toggle_window(self, feed_name, visible):
		"""Toggles a comic."""
		global feeds
		
		if not feed_name in feeds.feeds:
			return
		
		print 1
		
		if not visible:
			print 2
			windows = filter(lambda w: w.feed_name == feed_name, self.windows)
			if not windows:
				return
			print 3
			
			for window in windows:
				print window
				window.close()
		
		else:
			template = Settings()
			template['feed_name'] = feed_name
			self.create_window(template = template)
	
	def update_icon(self):
		"""Update the icon of the applet, scaling it if necessary."""
		if self.height != self.icon.get_height():
			self.set_icon(self.icon.scale_simple(self.height, self.height,
				gtk.gdk.INTERP_BILINEAR))
		else:
			self.set_icon(self.icon)
	
	def make_menu(self):
		"""Generate the menu listing the comics."""
		global feeds
		
		# Generate comics menu
		feed_menu = self.__xml.get_widget('comics_menu')
		feed_menu.foreach(lambda child: feed_menu.remove(child))
		for feed in feeds.feeds:
			label = gtk.Label()
			label.set_markup(feeds.feeds[feed].name)
			align = gtk.Alignment(xalign = 0.0)
			align.add(label)
			menu_item = gtk.CheckMenuItem()
			menu_item.data = feed
			menu_item.add(align)
			menu_item.set_active(len(filter(lambda w: w.feed_name == feed,
				self.windows)) > 0)
			menu_item.connect('toggled', self.on_comics_toggled)
			feed_menu.append(menu_item)
		feed_menu.show_all()
		self.__xml.get_widget('comics_container').set_sensitive(
			len(feeds.feeds) > 0)
		
		return self.__xml.get_widget('menu')
	
	def __init__(self, uid, orient, height):
		global feeds
		
		awn.AppletSimple.__init__(self, uid, orient, height)
		
		self.height = height
		self.icon = gtk.gdk.pixbuf_new_from_file(join(SHARE_DIR, 'icon.svg'))
		self.update_icon()
		self.notify = awn.extras.AWNLib.Notify(self)
		self.notify.require()
		
		self.connect('destroy', self.on_destroy)
		self.connect('button-press-event', self.on_button_press)
		
		self.__xml = gtk.glade.XML(GLADE_FILE)
		self.__xml.signal_autoconnect(self)
		
		self.visible = False
		self.windows = []
		
		try:
			for filename in filter(lambda f: f.endswith('.strip'),
					os.listdir(STRIPS_DIR)):		
				strip = self.create_window(os.path.join(STRIPS_DIR, filename))
		except OSError:
			return
	
		for feed in feeds.feeds.values():
			feed.update()
	
	def on_window_updated(self, widget, title):
		self.notify.send(title,
			_('There is a new strip of %s!') % widget.feed_name,
			os.path.join(SHARE_DIR, 'icon.svg'))
	
	def on_window_removed(self, widget):
		self.windows.remove(widget)
	
	def on_destroy(self, widget):
		for window in self.windows:
			window.save_settings()
	
	def on_button_press(self, widget, event):
		if event.button == 1:
			self.set_visibility(not self.visible)
		elif event.button == 3:
			menu = self.make_menu()
			if menu:
				menu.popup(None, None, None, event.button, event.time)
		else:
			return False
		return True
	
	def on_comics_toggled(self, widget):
		self.toggle_window(widget.data, widget.get_active())
	
	def on_manage_comics_activated(self, widget):
		manager = comics_manage.ComicsManager()
		manager.show()


if __name__ == '__main__':
	# Initialise internationalisation
	locale.setlocale(locale.LC_ALL, '')
	locale.bindtextdomain('comics', LOCALE_DIR)
	locale.textdomain('comics')
	
	# Initialise threading
	gobject.threads_init()
	gtk.gdk.threads_init()
	
	#Initialise AWN and create the applet
	awn.init(sys.argv[1:])
	applet = ComicApplet(awn.uid, awn.orient, awn.height)
	awn.init_applet(applet)
	applet.show_all()
	
	gtk.main()
