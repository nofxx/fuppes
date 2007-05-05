/* trashapplet from gnome-applets-2.18.0
 * 
 * Copyright (c) 2004  Michiel Sikkes <michiel@eyesopened.nl>,
 *               2004  Emmanuele Bassi <ebassi@gmail.com>
*/

#ifndef _FUPPES_APPLET_H_
#define _FUPPES_APPLET_H_

#include <panel-applet.h>
#include <gnome.h>

#define FUPPES_TYPE_APPLET (fuppes_applet_get_type ())
#define FUPPES_APPLET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), FUPPES_TYPE_APPLET, FuppesApplet))
#define FUPPES_APPLET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), FUPPES_TYPE_APPLET, FuppesAppletClass))
#define FUPPES_IS_APPLET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FUPPES_TYPE_APPLET))
#define FUPPES_IS_APPLET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FUPPES_TYPE_APPLET))
#define FUPPES_APPLET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), FUPPES_TYPE_APPLET, FuppesAppletClass))

/*#define TRASH_ICON_EMPTY	"user-trash"
#define TRASH_ICON_EMPTY_ACCEPT "user-trash"
#define TRASH_ICON_FULL		"user-trash-full"

typedef enum {
	TRASH_STATE_UNKNOWN,
	TRASH_STATE_EMPTY,
	TRASH_STATE_FULL,
	TRASH_STATE_ACCEPT
} TrashState;*/

typedef struct _FuppesApplet	 FuppesApplet;
typedef struct _FuppesAppletClass FuppesAppletClass;
struct _FuppesApplet
{
	PanelApplet applet;

	guint size;
	//guint new_size;
	PanelAppletOrient orient;

	GtkTooltips *tooltips;
	GtkWidget   *image;
	//TrashState icon_state;

  gboolean is_started;
	//gint item_count;
	//gboolean is_empty;
	//gboolean drag_hover;

	//GladeXML *xml;

	/*TrashMonitor *monitor;
	guint monitor_signal_id;

	guint update_id;*/
};

struct _FuppesAppletClass {
	PanelAppletClass parent_class;
};

#endif // _TRASH_APPLET_H_
