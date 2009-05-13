/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
 
 
 /* awn-CPUicon.c */

#include <glibtop/cpu.h>

#include "cpuicon.h"
#include "areagraph.h"

#include "sysmoniconprivate.h"

G_DEFINE_TYPE (AwnCPUicon, awn_CPUicon, AWN_TYPE_SYSMONICON)

#define AWN_CPUICON_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), AWN_TYPE_CPUICON, AwnCPUiconPrivate))

typedef struct _AwnCPUiconPrivate AwnCPUiconPrivate;

enum
{
  CPU_TOTAL,
  CPU_USED,
  N_CPU_STATES
};

struct _AwnCPUiconPrivate 
{
    guint timer_id;
    guint update_timeout;
    guint num_cpus;
    guint now; /*toggle used for the times*/
    guint64 times[2][GLIBTOP_NCPU][N_CPU_STATES];
};

static void
awn_CPUicon_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
awn_CPUicon_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
awn_CPUicon_dispose (GObject *object)
{
  G_OBJECT_CLASS (awn_CPUicon_parent_class)->dispose (object);
}

static void
awn_CPUicon_finalize (GObject *object)
{
  G_OBJECT_CLASS (awn_CPUicon_parent_class)->finalize (object);
}

static gboolean 
_awn_CPUicon_update_icon(gpointer object)
{  
  AwnCPUiconPrivate * priv;  
  AwnSysmoniconPrivate * sysmonicon_priv=NULL;  
  AwnCPUicon * icon = object;
  
  priv = AWN_CPUICON_GET_PRIVATE (object);
  sysmonicon_priv = AWN_SYSMONICON_GET_PRIVATE (object);
  
  g_debug ("Fire!\n");

  //  awn_graph_add_data (awn_sysmonicon_get_graph(AWN_SYSMONICON(self)),&point);
  awn_graph_add_data (sysmonicon_priv->graph,NULL);
  awn_sysmonicon_update_icon (icon);
  return TRUE;
}

static void
awn_CPUicon_constructed (GObject *object)
{
  AwnCPUiconPrivate * priv;
  AwnSysmoniconPrivate * sysmonicon_priv=NULL;  
  glibtop_cpu cpu;
  int i = 0;
    
  priv = AWN_CPUICON_GET_PRIVATE (object);
  priv->timer_id = g_timeout_add(priv->update_timeout, _awn_CPUicon_update_icon, object);  
  
  priv->num_cpus = 0;
  glibtop_get_cpu(&cpu);

  while (i < GLIBTOP_NCPU && cpu.xcpu_total[i] != 0)
  {
    priv->num_cpus++;
    i++;
  }
  priv->now = 0;
  

  sysmonicon_priv = AWN_SYSMONICON_GET_PRIVATE (object);

  sysmonicon_priv->graph = AWN_GRAPH(awn_areagraph_new (48,0.0,100.0));
  
  
}

static void
awn_CPUicon_class_init (AwnCPUiconClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (AwnCPUiconPrivate));

  object_class->get_property = awn_CPUicon_get_property;
  object_class->set_property = awn_CPUicon_set_property;
  object_class->dispose = awn_CPUicon_dispose;
  object_class->finalize = awn_CPUicon_finalize;
  object_class->constructed = awn_CPUicon_constructed;
}


static void
awn_CPUicon_init (AwnCPUicon *self)
{
  GdkPixbuf * pixbuf;  
  AwnCPUiconPrivate *priv;
  	
  priv = AWN_CPUICON_GET_PRIVATE (self);
  priv->update_timeout = 1000;
}

GtkWidget*
awn_CPUicon_new (AwnApplet * applet)
{
  GtkWidget * cpuicon = NULL;
  cpuicon = g_object_new (AWN_TYPE_CPUICON,
                          "applet",applet,
                          NULL);
  return cpuicon;
}

static void
get_load(AwnCPUicon *self)
{
  guint i;
  glibtop_cpu cpu;
  AwnCPUiconPrivate *priv;
  float  total, used;
  gdouble load;
  Awn_AreagraphPoint point;
  
  priv = AWN_CPUICON_GET_PRIVATE (self);

  glibtop_get_cpu(&cpu);

#undef NOW
#undef LAST
#define NOW  (priv->times[priv->now])
#define LAST (priv->times[priv->now ^ 1])

  if (priv->num_cpus == 1)
  {
    NOW[0][CPU_TOTAL] = cpu.total;
    NOW[0][CPU_USED] = cpu.user + cpu.nice + cpu.sys;
  }
  else
  {
    for (i = 0; i < priv->num_cpus; i++)
    {
      NOW[i][CPU_TOTAL] = cpu.xcpu_total[i];
      NOW[i][CPU_USED] = cpu.xcpu_user[i] + cpu.xcpu_nice[i] + cpu.xcpu_sys[i];
    }
  }

  load = total = used = 0.0;

  for (i = 0; i < priv->num_cpus; i++)
  {
    total = total + NOW[i][CPU_TOTAL] - LAST[i][CPU_TOTAL];
    used  = used + NOW[i][CPU_USED]  - LAST[i][CPU_USED];
  }

  load = used / MAX(total, (float)priv->num_cpus * 1.0f);

  point.value = load;
  point.points = 1.0;   /*FIXME... do a proper calc... timeouts are NOT exact*/

  // toggle the buffer index.
  priv->now ^= 1;

#undef NOW
#undef LAST
}


