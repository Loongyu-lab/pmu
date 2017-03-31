/* pmu-spi.c
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

#include "c37/c37.h"
#include "pmu-app.h"
#include "pmu-window.h"

#include "pmu-spi.h"

#define DATA_SIZE 64 /* Bytes */

struct _PmuSpi
{
  GObject parent_instance;

  GMainContext   *context;
};

GThread *spi_thread  = NULL;
PmuSpi  *default_spi = NULL;

G_DEFINE_TYPE (PmuSpi, pmu_spi, G_TYPE_OBJECT)

enum {
  SPI_DATA_BEGIN,    /* After 0xFFFF test data has been received from FPGA */
  START_SPI,
  STOP_SPI,
  SPI_DATA_RECEIVED,
  N_SIGNALS,
};

static guint signals[N_SIGNALS] = { 0, };

GQueue *spi_data;

static void
pmu_spi_finalize (GObject *object)
{
  G_OBJECT_CLASS (pmu_spi_parent_class)->finalize (object);
}

static void
pmu_spi_class_init (PmuSpiClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = pmu_spi_finalize;

  signals [START_SPI] =
    g_signal_new ("start-spi",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL,
                  G_TYPE_NONE,
                  0);

  signals [START_SPI] =
    g_signal_new ("stop-spi",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL,
                  G_TYPE_NONE,
                  0);
}

static void
pmu_spi_init (PmuSpi *self)
{
}

PmuSpi *
pmu_spi_get_default (void)
{
  return default_spi;
}

GMainContext *
pmu_spi_get_default_context (void)
{
  return default_spi->context;
}

static void
start_spi_cb (PmuSpi   *self,
              gpointer  user_data)
{
  g_autoptr(GError) error = NULL;

  g_usleep (1000 * 1000 * 10);
}

static void
stop_spi_cb (PmuSpi   *self,
             gpointer  user_data)
{
  g_autoptr(GError) error = NULL;
}

static void
pmu_spi_new (gpointer user_data)
{
  g_autoptr(GError) error = NULL;
  g_autoptr(GMainContext) spi_context = NULL;
  g_autoptr(GMainLoop) spi_loop = NULL;

  spi_context = g_main_context_new ();
  spi_loop = g_main_loop_new(spi_context, FALSE);

  g_main_context_push_thread_default (spi_context);

  default_spi = g_object_new (PMU_TYPE_SPI, NULL);
  default_spi->context = spi_context;

  g_signal_connect (default_spi, "start-spi",
                    G_CALLBACK (start_spi_cb), user_data);

  g_signal_connect (default_spi, "stop-spi",
                    G_CALLBACK (stop_spi_cb), user_data);

  g_main_loop_run(spi_loop);

  g_main_context_pop_thread_default (spi_context);
}

void
pmu_spi_start_thread (gpointer user_data)
{
  g_autoptr(GError) error = NULL;

  if (spi_thread == NULL)
    {
      spi_thread = g_thread_try_new ("spi",
                                     (GThreadFunc)pmu_spi_new,
                                     user_data,
                                     &error);
      if (error != NULL)
        g_warning ("Cannot create spi thread. Error: %s", error->message);
    }
}

gboolean
pmu_spi_start (gpointer user_data)
{
  if (spi_thread == NULL)
    pmu_spi_start_thread (NULL);

  if (default_spi)
    g_signal_emit_by_name (default_spi, "start-spi");

  return G_SOURCE_REMOVE;
}

gboolean
pmu_spi_stop (gpointer user_data)
{
  g_signal_emit_by_name (default_spi, "stop-spi");

  return G_SOURCE_REMOVE;
}
