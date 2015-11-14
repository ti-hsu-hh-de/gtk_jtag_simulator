#include <gtk/gtk.h>
#include "jtag_tap.h"
#include <stdio.h>
#include <stdlib.h>

#define APP_NAME  "JTAG Simulator"

struct jtag_tap *TAP;
char tms=0;
char tck=0;
char tdi=0;

static void jtag_toggle_tms(GtkWidget *widget, gpointer data) {
  if (tms==0) {
    tms=1;
  } else {
    tms=0;
  }
}

static void jtag_toggle_tdi(GtkWidget *widget, gpointer data) {
  if (tdi==0) {
    tdi=1;
  } else {
    tdi=0;
  }
}

static void jtag_toggle_tck(GtkWidget *widget, gpointer data) {
  if (tck==0) {
    tck=1;
  } else {
    tck=0;
  }
}

static void jtag_start(GtkWidget *widget, gpointer data) {
  GtkBuilder *builder=GTK_BUILDER(data);
  GObject *obj;
  GObject *state;
  GtkStyleContext *context;
  const char *help;
  const char *xilinx_data;
  int steps=0;
  int i;

  //get number of steps to do
  obj=gtk_builder_get_object (builder, "jtag_control_steps");
  help=gtk_entry_get_text (GTK_ENTRY(GTK_WIDGET(obj)));
  steps=atoi(help);

  printf("Steps: %d\n",steps);

  //get xilinx data
  obj=gtk_builder_get_object (builder, "jtag_control_data");
  xilinx_data=gtk_entry_get_text (GTK_ENTRY(GTK_WIDGET(obj)));

  printf("Xilinx Data: %s\n",xilinx_data);

  jtag_shift_xilinx(TAP, xilinx_data, steps);


  for(i=0; i<JTAG_STATES_NR; i++) {
    state=gtk_builder_get_object (builder, state_ids[i]);
    context=gtk_widget_get_style_context (GTK_WIDGET(state));
    gtk_style_context_remove_class (context, "TAP_ACTIVE");
  }

  state=gtk_builder_get_object (builder, (char *)jtag_tap_get_state_id(TAP));
  context=gtk_widget_get_style_context (GTK_WIDGET(state));
  gtk_style_context_add_class (context, "TAP_ACTIVE");

}



static void jtag_tick(GtkWidget *widget, gpointer data) {
  GtkBuilder *builder=GTK_BUILDER(data);
  GObject *state;
  GtkStyleContext *context;
  int i;

  jtag_tap_shift(TAP,&tms, &tdi, &tck, 1);

  for(i=0; i<JTAG_STATES_NR; i++) {
    state=gtk_builder_get_object (builder, state_ids[i]);
    context=gtk_widget_get_style_context (GTK_WIDGET(state));
    gtk_style_context_remove_class (context, "TAP_ACTIVE");
  }

  state=gtk_builder_get_object (builder, (char *)jtag_tap_get_state_id(TAP));
  context=gtk_widget_get_style_context (GTK_WIDGET(state));
  gtk_style_context_add_class (context, "TAP_ACTIVE");


}

int main (int argc, char **argv) {
        GdkDisplay		*display;
	      GdkScreen		*screen;
        GObject *window;
        GtkCssProvider		*provider;
        GFile			*file;
        GtkStyleContext *context;
        GObject *state;
        GtkBuilder *builder;
        GObject *button_next_step;
        GObject *checkbox;
        GObject *button;

        TAP=jtag_tap_init();
        gtk_init (&argc, &argv);

        display = gdk_display_get_default();
	      screen = gdk_display_get_default_screen(display);
        provider = gtk_css_provider_new();
        file = g_file_new_for_path("jtag_simulator.css");
        gtk_css_provider_load_from_file(provider, file, NULL);
        gtk_style_context_add_provider_for_screen(screen,GTK_STYLE_PROVIDER(provider),GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

        /* Construct a GtkBuilder instance and load our UI description */
        builder = gtk_builder_new ();
        gtk_builder_add_from_file (builder, "jtag_simulator.glade", NULL);

        /* Connect signal handlers to the constructed widgets. */
        window = gtk_builder_get_object (builder, "jtag_simulator_main_window");
        g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

        state=gtk_builder_get_object (builder, (char *)jtag_tap_get_state_id(TAP));
        context=gtk_widget_get_style_context (GTK_WIDGET(state));
        gtk_style_context_add_class (context, "TAP_ACTIVE");

        button_next_step=gtk_builder_get_object (builder,"jtag_button_step");
        g_signal_connect (button_next_step, "clicked", G_CALLBACK (jtag_tick), builder);

        checkbox=gtk_builder_get_object (builder,"control_tms");
        g_signal_connect (checkbox, "clicked", G_CALLBACK (jtag_toggle_tms), NULL);
        checkbox=gtk_builder_get_object (builder,"control_tdi");
        g_signal_connect (checkbox, "clicked", G_CALLBACK (jtag_toggle_tdi), NULL);
        checkbox=gtk_builder_get_object (builder,"control_tck");
        g_signal_connect (checkbox, "clicked", G_CALLBACK (jtag_toggle_tck), NULL);

        button=gtk_builder_get_object (builder,"jtag_control_start");
        g_signal_connect (button, "clicked", G_CALLBACK (jtag_start), builder);

        gtk_main ();


        return 0;
}
