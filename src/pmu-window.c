/* pmu-window.c
 *
 * Copyright (C) 2017 Mohammed Sadiq <sadiq@sadiqpk.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "pmu-window.h"


struct _PmuWindow
{
  GtkApplicationWindow parent_instance;

  GtkWidget *start_button;
  GtkWidget *stop_button;
  GtkWidget *menu_button;
  GtkWidget *info_label;
  GtkWidget *revealer;

  guint revealer_timeout_id;
};

static GThread *ntp_thread;


G_DEFINE_TYPE (PmuWindow, pmu_window, GTK_TYPE_APPLICATION_WINDOW)

static void sync_ntp_time_cb (GSimpleAction       *action,
                              GVariant            *param,
                              gpointer             user_data);

static const GActionEntry win_entries[] = {
  { "sync-ntp",  sync_ntp_time_cb },
};

static gboolean
revealer_timeout (gpointer user_data)
{
  PmuWindow *window = PMU_WINDOW (user_data);

  if (window->revealer_timeout_id)
    {
      g_source_remove (window->revealer_timeout_id);
      window->revealer_timeout_id = 0;
    }

  gtk_revealer_set_reveal_child (GTK_REVEALER (window->revealer), FALSE);
  return G_SOURCE_REMOVE;
}

static gboolean
show_ntp_update_revealer (gpointer user_data)
{
  PmuWindow *window = PMU_WINDOW (user_data);

  gtk_label_set_label (GTK_LABEL (window->info_label), "NTP Sync completed");
  gtk_revealer_set_reveal_child (GTK_REVEALER (window->revealer), TRUE);
  window->revealer_timeout_id = g_timeout_add_seconds (5, revealer_timeout, window);

  return FALSE;
}

static gpointer
sync_ntp_time (gpointer user_data)
{
  GError *error = NULL;
  gint exit_status = 1;

  if (!g_spawn_command_line_sync ("/usr/bin/pkexec good.sh",
                                  NULL, NULL,
                                  &exit_status,
                                  &error))
    {
      g_warning ("%s", error->message);
      g_clear_error (&error);
      return NULL;
    }

  if (exit_status == 0)
    g_idle_add (show_ntp_update_revealer,
                user_data);
}

static void sync_ntp_time_cb (GSimpleAction *action,
                              GVariant      *param,
                              gpointer       user_data)
{
  ntp_thread = g_thread_new (NULL,
                             sync_ntp_time,
                             user_data);
}

static void
pmu_window_finalize (GObject *object)
{
  G_OBJECT_CLASS (pmu_window_parent_class)->finalize (object);
}

static void
pmu_window_constructed (GObject *object)
{
  PmuWindow  *window;
  GMenuModel *menu;
  GAction    *action;
  g_autoptr(GtkBuilder) builder = NULL;

  window = PMU_WINDOW (object);

  builder = gtk_builder_new_from_resource ("/org/sadiqpk/pmu/gtk/menus.ui");
  menu = G_MENU_MODEL (gtk_builder_get_object (builder, "win-menu"));
  gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (window->menu_button), menu);

  G_OBJECT_CLASS (pmu_window_parent_class)->constructed (object);
}

static void
start_button_clicked_cb (GtkWidget *button,
                         PmuWindow *window)
{
  gtk_label_set_label (GTK_LABEL (window->info_label), "PMU Server started successfully");
  gtk_revealer_set_reveal_child (GTK_REVEALER (window->revealer), TRUE);
  window->revealer_timeout_id = g_timeout_add_seconds (5, revealer_timeout, window);

  gtk_widget_hide (window->start_button);
  gtk_widget_show (window->stop_button);
}

static void
revealer_button_clicked_cb (GtkWidget *button,
                            PmuWindow *window)
{
  gtk_revealer_set_reveal_child (GTK_REVEALER (window->revealer), FALSE);
}

static void
stop_button_clicked_cb (GtkWidget *button,
                        PmuWindow *window)
{
  gtk_label_set_label (GTK_LABEL (window->info_label), "PMU Server stopped");
  gtk_revealer_set_reveal_child (GTK_REVEALER (window->revealer), TRUE);
  window->revealer_timeout_id = g_timeout_add_seconds (5, revealer_timeout, window);

  gtk_widget_hide (window->stop_button);
  gtk_widget_show (window->start_button);
}

static void
pmu_window_class_init (PmuWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = pmu_window_finalize;
  object_class->constructed = pmu_window_constructed;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/sadiqpk/pmu/ui/pmu-window.ui");

  gtk_widget_class_bind_template_child (widget_class, PmuWindow, menu_button);
  gtk_widget_class_bind_template_child (widget_class, PmuWindow, start_button);
  gtk_widget_class_bind_template_child (widget_class, PmuWindow, stop_button);
  gtk_widget_class_bind_template_child (widget_class, PmuWindow, info_label);
  gtk_widget_class_bind_template_child (widget_class, PmuWindow, revealer);

  gtk_widget_class_bind_template_callback (widget_class, start_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, stop_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, revealer_button_clicked_cb);
}

static void
pmu_window_init (PmuWindow *self)
{
  g_action_map_add_action_entries (G_ACTION_MAP (self),
                                   win_entries,
                                   G_N_ELEMENTS (win_entries),
                                   self);

  gtk_widget_init_template (GTK_WIDGET (self));
}

PmuWindow *
pmu_window_new (PmuApp *app)
{
  g_assert (GTK_IS_APPLICATION (app));

  return g_object_new (PMU_TYPE_WINDOW,
                       "application", app,
                       NULL);
}
