#include <string.h>

#include <gnome.h>
#include <panel-applet.h>
//#include <gtk/gtklabel.h>


static const char Context_menu_xml [] =
   "<popup name=\"button3\">\n"
   "   <menuitem name=\"StartStop Item\" "
   "             verb=\"FuppesStartStop\" "
   "           _label=\"_Start\"\n"
   "          pixtype=\"stock\" "
   "          pixname=\"gtk-refresh\"/>\n"
   "   <separator/>\n"
   
   "   <menuitem name=\"About Item\" "
   "             verb=\"FuppesAbout\" "
   "           _label=\"_About...\"\n"
   "          pixtype=\"stock\" "
   "          pixname=\"gnome-stock-about\"/>\n"
   "</popup>\n";


/*
"   <menuitem name=\"Properties Item\" "
   "             verb=\"ExampleProperties\" "
   "           _label=\"_Preferences...\"\n"
   "          pixtype=\"stock\" "
   "          pixname=\"gtk-properties\"/>\n"
   */

static void start_stop_fuppes(BonoboUIComponent *uic, PanelApplet *applet, const gchar       *verbname) {
  //	
}


static void
display_about_dialog (BonoboUIComponent *uic,
                                 PanelApplet *applet, const gchar       *verbname) {
	
	static const gchar *authors [] = {
		"Dave Camp <campd@oit.edu>",
		NULL
	};

	const gchar *documenters[] = {
                "Arjan Scherpenisse <acscherp@wins.uva.nl>",
                "Telsa Gwynne <hobbit@aloss.ukuu.org.uk>",
                "Sun GNOME Documentation Team <gdocteam@sun.com>",
		NULL
	};
	
	gtk_show_about_dialog (NULL,
		"name",		_("Fuppes"),
		"version",	"0.7.2-dev",
		"comments",	_("The FUPPES Gnome Panel Applet"),
		"copyright",	"\xC2\xA9 2007 Ulrich Völkel",
		"authors",	authors,
		"documenters",	documenters,
		"translator-credits",	_("translator-credits"),
		"logo-icon-name",	"fuppes",
		NULL);
}


static const BonoboUIVerb fuppes_menu_verbs [] = {
        //BONOBO_UI_VERB ("ExampleProperties", display_properties_dialog),
        BONOBO_UI_VERB ("FuppesStartStop", start_stop_fuppes),
        BONOBO_UI_VERB ("FuppesAbout", display_about_dialog),
        BONOBO_UI_VERB_END
};



static gboolean on_button_press (GtkWidget      *event_box, 
                         GdkEventButton *event,
                         gpointer        data)
{
	static int window_shown;
	static GtkWidget *window, *box, *image, *label;
	/* Don't react to anything other than the left mouse button;
	   return FALSE so the event is passed to the default handler */
	if (event->button != 1)
		return FALSE;

	if (!window_shown) {
		window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		box = GTK_BOX (gtk_vbox_new (TRUE, 12));
		gtk_container_add (GTK_CONTAINER (window), box);

		image = GTK_IMAGE (gtk_image_new_from_file ("/usr/share/pixmaps/fuppes.svg"));
		gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 12);

		label = gtk_label_new ("Hello World");
		gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 12);
		
		gtk_widget_show_all (window);
	}
	else
		gtk_widget_hide (GTK_WIDGET (window));

	window_shown = !window_shown;
	return TRUE;
}

int nSize = 0;

static void size_allocate_cb(PanelApplet *w, GtkAllocation *allocation, gpointer data)
{
  nSize = allocation->width;
  if(allocation->width > allocation->height)
    nSize = allocation->height;
}

static gboolean fuppes_applet_factory(PanelApplet *applet, const gchar *iid, gpointer data)
{
	GtkWidget   *image, *box;
  GtkTooltips *tooltips;
  GtkAllocation* allocation;

	if (strcmp (iid, "OAFIID:FuppesApplet") != 0)
		return FALSE;

  
  //panel_applet_get_size(applet)

  panel_applet_set_flags(applet, PANEL_APPLET_EXPAND_MINOR);
  //panel_applet_set_size_hints(applet, NULL, 1, 22);

  //gtk_window_set_default_size(applet, 22, 22);

	// load image
	image = gtk_image_new_from_file("/usr/share/pixmaps/fuppes.png");
  
  /*box = gtk_vbox_new(FALSE, 0);
  allocation->width  = 22;
  allocation->height = 22;
  gtk_widget_size_allocate(image, allocation);
  
  gtk_container_add(GTK_CONTAINER(box), image);*/
  
  gtk_container_add(GTK_CONTAINER(applet), image);

  // connect signal handler
  g_signal_connect(G_OBJECT(applet), 
                   "button_press_event",
                   G_CALLBACK(on_button_press),
                   applet);                  
  g_signal_connect(G_OBJECT(applet),
                   "size_allocate",
                   G_CALLBACK(size_allocate_cb),
                   applet);                  

  // create tooltips
  tooltips = gtk_tooltips_new();
  gtk_tooltips_set_tip(tooltips, 
                       GTK_WIDGET(applet),
                       _("FUPPES UPnP Server"),
                       NULL);

  // setup context menu
  panel_applet_setup_menu(PANEL_APPLET(applet),
                          Context_menu_xml,
                          fuppes_menu_verbs,
                          NULL);

	gtk_widget_show_all(GTK_WIDGET(applet));

	return TRUE;
}



PANEL_APPLET_BONOBO_FACTORY("OAFIID:FuppesApplet_Factory", 
                            PANEL_TYPE_APPLET,
                            "The Fuppes Applet",
                            "0",
                            fuppes_applet_factory,
                            NULL);
