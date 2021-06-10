#include <stdio.h>
#include <gtk/gtk.h>

/**
 *
 * @param widget
 * @param data
 */
static void print_hello(GtkWidget *widget, gpointer data) {
  g_print("Hello World\n");
}

/**
 *
 * @param app           GTK 应用
 * @param user_data     用户数据
 */
static void activate(GtkApplication *app, gpointer user_data) {
  // 创建窗口，设置标题等（Widget 实际上是 Window 的父类，但 C 没有面向对象，所以在此需要强制转换）。
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "HelloWorld");
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

  // 创建按钮和容器。
  GtkButtonBox *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add(GTK_CONTAINER(window), button_box);
  GtkButton *button = gtk_button_new_with_label("Hello World");
  gtk_container_add(GTK_CONTAINER(button_box), button);

  g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);
  g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_window_close), window);

  gtk_widget_show_all(window);
}

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {

  // 创建应用。
  GtkApplication *app = gtk_application_new("com.ywh.clang", G_APPLICATION_FLAGS_NONE);

  // 绑定信号与回调函数。
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(app, argc, argv);

  // 释放内存。
  g_object_unref(app);
  return status;
}
