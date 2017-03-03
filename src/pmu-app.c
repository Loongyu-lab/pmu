/* pmu-app.c
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
#include "pmu-setup-window.h"

#include "pmu-app.h"


struct _PmuApp
{
  GtkApplication parent_instance;

  GSettings *settings;
};


G_DEFINE_TYPE (PmuApp, pmu_app, GTK_TYPE_APPLICATION)

static void pmu_app_show_about (GSimpleAction *action,
                                GVariant      *parameter,
                                gpointer       user_data);

static void pmu_app_quit       (GSimpleAction *action,
                                GVariant      *parameter,
                                gpointer       user_data);


static const GActionEntry app_entries[] = {
  { "about",  pmu_app_show_about },
  { "quit",   pmu_app_quit       },
};

static void pmu_app_show_about (GSimpleAction *action,
                                GVariant      *parameter,
                                gpointer       user_data)
{
  PmuApp      *self;
  const gchar *authors[] = {
    "Mohammed Sadiq <sadiq@sadiqpk.org>",
    NULL
  };

  self = PMU_APP (user_data);

  g_assert (G_IS_APPLICATION (self));
  g_assert (G_IS_SIMPLE_ACTION (action));

  gtk_show_about_dialog (gtk_application_get_active_window (GTK_APPLICATION (self)),
                         "program-name", ("PMU"),
                         "version", "0.1.0",
                         "copyright", "Copyright \xC2\xA9 2017 Mohammed Sadiq",
                         "license-type", GTK_LICENSE_GPL_3_0,
                         "authors", authors,
                         // "artists", authors,
                         "logo-icon-name", "org.gnome.Todo",
                         "translator-credits", ("translator-credits"),
                         NULL);
}

static void pmu_app_quit (GSimpleAction *action,
                          GVariant      *parameter,
                          gpointer       user_data)
{
  GApplication *app = user_data;

  g_assert (G_IS_APPLICATION (app));
  g_assert (G_IS_SIMPLE_ACTION (action));

  g_application_quit (app);
}

static void
pmu_app_finalize (GObject *object)
{
  G_OBJECT_CLASS (pmu_app_parent_class)->finalize (object);
}

static void
pmu_app_activate (GApplication *app)
{
  GtkWidget *window;
  gboolean   first_run;

  g_assert (GTK_IS_APPLICATION (app));

  first_run = g_settings_get_boolean (PMU_APP (app)->settings, "first-run");

  if (first_run)
    {
      window = GTK_WIDGET (pmu_setup_window_new (PMU_APP (app)));
      gtk_window_present (GTK_WINDOW (window));
    }
  else
    {
      window = GTK_WIDGET (gtk_application_get_active_window (GTK_APPLICATION (app)));

      if (window == NULL)
        window = GTK_WIDGET (pmu_window_new (PMU_APP (app)));
    }

  gtk_window_present (GTK_WINDOW (window));
}

static void
pmu_app_startup (GApplication *app)
{
  g_action_map_add_action_entries (G_ACTION_MAP (app),
                                   app_entries,
                                   G_N_ELEMENTS (app_entries),
                                   app);

  G_APPLICATION_CLASS (pmu_app_parent_class)->startup (app);
}

static void
pmu_app_class_init (PmuAppClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GApplicationClass *application_class = G_APPLICATION_CLASS (klass);

  object_class->finalize = pmu_app_finalize;

  application_class->activate = pmu_app_activate;
  application_class->startup = pmu_app_startup;
}

static void
pmu_app_init (PmuApp *self)
{
  self->settings = g_settings_new ("org.sadiqpk.pmu");
}

PmuApp *
pmu_app_new (void)
{
  return g_object_new (PMU_TYPE_APP,
                       "application-id", "org.sadiqpk.pmu",
                       "flags", G_APPLICATION_FLAGS_NONE,
                       NULL);
}
