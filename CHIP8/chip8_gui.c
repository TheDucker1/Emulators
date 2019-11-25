/*****************************************************
chip8.h: GUI for chip8.c
Author: TheDucker1
GUI Toolkit: GTK+ 2.0
-BLINKY
*****************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#include<cairo.h>
#include<gtk/gtk.h>

#include"chip8.h"

#define SCALE 15
#define UPDATE_TIME 5

static gboolean key_pressed(GtkWidget* widget, GdkEventKey *event, void ** data) {
  CHIP8* ref = data[1];
  if ((event->keyval & 0xFF00) >> 8 != 0xFF) {
    switch (event->keyval) {
      case 'x': ref->key[0x0] = 1;
      break;
      
      case '1': ref->key[0x1] = 1;
      break;
      
      case '2': ref->key[0x2] = 1;
      break;
      
      case '3': ref->key[0x3] = 1;
      break;
      
      case 'q': ref->key[0x4] = 1;
      break;
      
      case 'w': ref->key[0x5] = 1;
      break;
      
      case 'e': ref->key[0x6] = 1;
      break;
      
      case 'a': ref->key[0x7] = 1;
      break;
      
      case 's': ref->key[0x8] = 1;
      break;
      
      case 'd': ref->key[0x9] = 1;
      break;
      
      case 'z': ref->key[0xA] = 1;
      break;
      
      case 'c': ref->key[0xB] = 1;
      break;
      
      case '4': ref->key[0xC] = 1;
      break;
      
      case 'r': ref->key[0xD] = 1;
      break;
      
      case 'f': ref->key[0xE] = 1;
      break;
      
      case 'v': ref->key[0xF] = 1;
      break;      
    }
    return TRUE;
  }
  return FALSE;
}


static gboolean key_released(GtkWidget* widget, GdkEventKey *event, void ** data) {
  CHIP8* ref = data[1];
  if ((event->keyval & 0xFF00) >> 8 != 0xFF) {
    switch (event->keyval) {
      case 'x': ref->key[0x0] = 0;
      break;
      
      case '1': ref->key[0x1] = 0;
      break;
      
      case '2': ref->key[0x2] = 0;
      break;
      
      case '3': ref->key[0x3] = 0;
      break;
      
      case 'q': ref->key[0x4] = 0;
      break;
      
      case 'w': ref->key[0x5] = 0;
      break;
      
      case 'e': ref->key[0x6] = 0;
      break;
      
      case 'a': ref->key[0x7] = 0;
      break;
      
      case 's': ref->key[0x8] = 0;
      break;
      
      case 'd': ref->key[0x9] = 0;
      break;
      
      case 'z': ref->key[0xA] = 0;
      break;
      
      case 'c': ref->key[0xB] = 0;
      break;
      
      case '4': ref->key[0xC] = 0;
      break;
      
      case 'r': ref->key[0xD] = 0;
      break;
      
      case 'f': ref->key[0xE] = 0;
      break;
      
      case 'v': ref->key[0xF] = 0;
      break;      
    }
    return TRUE;
  }
  return FALSE;
}


static gboolean update(void ** data) {
  GtkWidget *widget = data[0];
  CHIP8* ref = data[1];
  //printf("OPCODE: %04x\tPC: %x\tI: %x\n", ref->opcode, ref->pc, ref->I); //debug
  if (widget->window == NULL) return FALSE;
  //run a cycle
  ref->emulateCycle(ref);
  gtk_widget_queue_draw(widget);
  return TRUE;
}

static gboolean draw(GtkWidget *widget,
    GdkEventExpose *event, void ** data) {
  
  CHIP8* ref = data[1];
  cairo_t *cr;
  cr = gdk_cairo_create(widget->window);
    for (unsigned char y = 0; y < 32; ++y) {
      for (unsigned char x = 0; x < 64; ++x) {
        if (ref->gfx[x + y*64] != 0) {cairo_set_source_rgb (cr, 255, 255, 255);}
        else {cairo_set_source_rgb (cr, 0, 0, 0);}
        cairo_rectangle(cr, x * SCALE, y * SCALE, (x+1)*SCALE, (y+1)*SCALE);
        cairo_fill (cr);
        cairo_stroke (cr);
      }
    }
  cairo_destroy(cr);

    /*cr = gdk_cairo_create(widget->window);

    cairo_move_to(cr, 30, 30);
    cairo_set_font_size(cr, 15);
    cairo_show_text(cr, "HELLO");
    cairo_set_line_width (cr, 1);
    cairo_set_source_rgb (cr, 0, 255, 0);
    cairo_rectangle(cr, 0, 0, 100, 100);
    cairo_stroke (cr);
    cairo_destroy(cr);
    */
  return FALSE;
}

int main(int argc, char * argv[]) {
  if (argc == 1) {
    printf("File error: No input file\n");
    exit (1); //no input file
  }
  srand(time(NULL));
  CHIP8* mychip8 = CHIP8_init();
  mychip8->loadGame(mychip8, argv[1]); 

  //Setting up window
  GtkWidget *window;
  GtkWidget *dArea;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  dArea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), dArea);

  gtk_window_set_title(GTK_WINDOW(window), "CHIP8");
  gtk_widget_set_events(window, GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
  gtk_window_set_default_size(GTK_WINDOW(window), 64 * SCALE, 32 * SCALE);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

  void* callback_data[] = {window, mychip8};

  g_signal_connect(G_OBJECT(window), "key_press_event",
              G_CALLBACK(key_pressed), callback_data);
  g_signal_connect(G_OBJECT(window), "key_release_event",
              G_CALLBACK(key_released), callback_data);
  g_signal_connect(dArea, "expose-event",
              G_CALLBACK(draw), callback_data);
  g_timeout_add(UPDATE_TIME, update, callback_data);
  g_signal_connect(G_OBJECT(window), "destroy",
              G_CALLBACK(gtk_main_quit), NULL);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;

}
