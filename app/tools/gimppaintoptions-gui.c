/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include "config.h"

#include <gtk/gtk.h>

#include "libgimpwidgets/gimpwidgets.h"

#include "tools-types.h"

#include "base/temp-buf.h"

#include "core/gimpbrush.h"
#include "core/gimptoolinfo.h"

#include "paint/gimppaintoptions.h"

#include "widgets/gimppropwidgets.h"
#include "widgets/gimpviewablebox.h"
#include "widgets/gimpwidgets-constructors.h"
#include "widgets/gimpwidgets-utils.h"

#include "gimpairbrushtool.h"
#include "gimpclonetool.h"
#include "gimpconvolvetool.h"
#include "gimpdodgeburntool.h"
#include "gimperasertool.h"
#include "gimphealtool.h"
#include "gimpinktool.h"
#include "gimppaintoptions-gui.h"
#include "gimppenciltool.h"
#include "gimpperspectiveclonetool.h"
#include "gimpsmudgetool.h"
#include "gimptooloptions-gui.h"

#include "gimp-intl.h"




static GtkWidget * fade_options_gui           (GimpPaintOptions *paint_options,
                                               GType             tool_type);
static GtkWidget * gradient_options_gui       (GimpPaintOptions *paint_options,
                                               GType             tool_type,
                                               GtkWidget        *incremental_toggle);
static GtkWidget * jitter_options_gui         (GimpPaintOptions *paint_options,
                                               GType             tool_type);

static void gimp_paint_options_gui_reset_size (GtkWidget        *button,
                                               GimpPaintOptions *paint_options);


/*  public functions  */

GtkWidget *
gimp_paint_options_gui (GimpToolOptions *tool_options)
{
  GObject          *config  = G_OBJECT (tool_options);
  GimpPaintOptions *options = GIMP_PAINT_OPTIONS (tool_options);
  GtkWidget        *vbox    = gimp_tool_options_gui (tool_options);
  GtkWidget        *frame;
  GtkWidget        *table;
  GtkWidget        *menu;
  GtkWidget        *scale;
  GtkWidget        *label;
  GtkWidget        *button;
  GtkWidget        *incremental_toggle = NULL;
  gint              table_row          = 0;
  GType             tool_type;

  tool_type = tool_options->tool_info->tool_type;

  /*  the main table  */
  table = gtk_table_new (3, 4, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  g_object_set_data (G_OBJECT (vbox), GIMP_PAINT_OPTIONS_TABLE_KEY, table);

  /*  the paint mode menu  */
  menu  = gimp_prop_paint_mode_menu_new (config, "paint-mode", TRUE, FALSE);
  label = gimp_table_attach_aligned (GTK_TABLE (table), 0, table_row++,
                                     _("Mode:"), 0.0, 0.5,
                                     menu, 2, FALSE);

  if (tool_type == GIMP_TYPE_ERASER_TOOL     ||
      tool_type == GIMP_TYPE_CONVOLVE_TOOL   ||
      tool_type == GIMP_TYPE_DODGE_BURN_TOOL ||
      tool_type == GIMP_TYPE_SMUDGE_TOOL)
    {
      gtk_widget_set_sensitive (menu, FALSE);
      gtk_widget_set_sensitive (label, FALSE);
    }

  /*  the opacity scale  */
  scale = gimp_prop_opacity_spin_scale_new (config, "opacity",
                                            _("Opacity"));
  gtk_table_attach (GTK_TABLE (table), scale,
                    0, 3, table_row, table_row + 1,
                    GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_SHRINK, 0, 0);
  gtk_widget_show (scale);
  table_row++;

  /*  the brush  */
  if (g_type_is_a (tool_type, GIMP_TYPE_BRUSH_TOOL))
    {
      GtkObject *adj;
      GtkWidget *hbox;

      button = gimp_prop_brush_box_new (NULL, GIMP_CONTEXT (tool_options),
                                        _("Brush"), 2,
                                        "brush-view-type", "brush-view-size");
      gtk_table_attach (GTK_TABLE (table), button,
                        0, 3, table_row, table_row + 1,
                        GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_SHRINK, 0, 0);
      gtk_widget_show (button);
      table_row++;

      button = gimp_prop_dynamics_box_new (NULL, GIMP_CONTEXT (tool_options),
                                           _("Dynamics"), 2,
                                           "dynamics-view-type",
                                           "dynamics-view-size");
      gtk_table_attach (GTK_TABLE (table), button,
                        0, 3, table_row, table_row + 1,
                        GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_SHRINK, 0, 0);
      gtk_widget_show (button);
      table_row++;

      hbox = gtk_hbox_new (FALSE, 2);
      gtk_table_attach (GTK_TABLE (table), hbox,
                        0, 3, table_row, table_row + 1,
                        GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_SHRINK, 0, 0);
      gtk_widget_show (hbox);
      table_row++;

      scale = gimp_prop_spin_scale_new (config, "brush-size",
                                        _("Size"),
                                        0.01, 1.0, 2);
      gtk_box_pack_start (GTK_BOX (hbox), scale, TRUE, TRUE, 0);
      gtk_widget_show (scale);

      button = gimp_stock_button_new (GTK_STOCK_REFRESH, NULL);
      gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
      gtk_widget_show (button);

      g_signal_connect (button, "clicked",
                        G_CALLBACK (gimp_paint_options_gui_reset_size),
                        options);

      adj = gimp_prop_scale_entry_new (config, "brush-aspect-ratio",
                                       GTK_TABLE (table), 0, table_row++,
                                       /* Label for a slider that affects
                                          aspect ratio for brushes */
                                       _("Aspect:"),
                                       0.01, 0.1, 2,
                                       FALSE, 0.0, 0.0);
      gimp_scale_entry_set_logarithmic (adj, TRUE);

      button = gimp_prop_spin_scale_new (config, "brush-angle",
                                         _("Angle"),
                                         1.0, 5.0, 2);
      gtk_table_attach (GTK_TABLE (table), button,
                        0, 3, table_row, table_row + 1,
                        GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_SHRINK, 0, 0);
      gtk_widget_show (button);
      table_row++;
    }

  if (g_type_is_a (tool_type, GIMP_TYPE_BRUSH_TOOL))
    {
      frame = fade_options_gui (options, tool_type);
      gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
      gtk_widget_show (frame);

      frame = jitter_options_gui (options, tool_type);
      gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
      gtk_widget_show (frame);
    }

  /*  the "incremental" toggle  */
  if (tool_type == GIMP_TYPE_PENCIL_TOOL     ||
      tool_type == GIMP_TYPE_PAINTBRUSH_TOOL ||
      tool_type == GIMP_TYPE_ERASER_TOOL)
    {
      incremental_toggle =
        gimp_prop_enum_check_button_new (config,
                                         "application-mode",
                                         _("Incremental"),
                                         GIMP_PAINT_CONSTANT,
                                         GIMP_PAINT_INCREMENTAL);
      gtk_box_pack_start (GTK_BOX (vbox), incremental_toggle, FALSE, FALSE, 0);
      gtk_widget_show (incremental_toggle);
    }

  /* the "hard edge" toggle */
  if (tool_type == GIMP_TYPE_ERASER_TOOL            ||
      tool_type == GIMP_TYPE_CLONE_TOOL             ||
      tool_type == GIMP_TYPE_HEAL_TOOL              ||
      tool_type == GIMP_TYPE_PERSPECTIVE_CLONE_TOOL ||
      tool_type == GIMP_TYPE_CONVOLVE_TOOL          ||
      tool_type == GIMP_TYPE_DODGE_BURN_TOOL        ||
      tool_type == GIMP_TYPE_SMUDGE_TOOL)
    {
      button = gimp_prop_check_button_new (config, "hard", _("Hard edge"));
      gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
      gtk_widget_show (button);
    }

  if (g_type_is_a (tool_type, GIMP_TYPE_PAINTBRUSH_TOOL))
    {
      frame = gradient_options_gui (options, tool_type, incremental_toggle);
      gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
      gtk_widget_show (frame);
    }

  return vbox;
}


/*  private functions  */

static gboolean
tool_has_opacity_dynamics (GType tool_type)
{
  return (g_type_is_a (tool_type, GIMP_TYPE_PAINTBRUSH_TOOL) ||
          tool_type == GIMP_TYPE_CLONE_TOOL             ||
          tool_type == GIMP_TYPE_HEAL_TOOL              ||
          tool_type == GIMP_TYPE_PERSPECTIVE_CLONE_TOOL ||
          tool_type == GIMP_TYPE_DODGE_BURN_TOOL        ||
          tool_type == GIMP_TYPE_ERASER_TOOL);
}

static gboolean
tool_has_hardness_dynamics (GType tool_type)
{
  return (tool_type == GIMP_TYPE_AIRBRUSH_TOOL          ||
          tool_type == GIMP_TYPE_CLONE_TOOL             ||
          tool_type == GIMP_TYPE_HEAL_TOOL              ||
          tool_type == GIMP_TYPE_PERSPECTIVE_CLONE_TOOL ||
          tool_type == GIMP_TYPE_CONVOLVE_TOOL          ||
          tool_type == GIMP_TYPE_ERASER_TOOL            ||
          tool_type == GIMP_TYPE_DODGE_BURN_TOOL        ||
          tool_type == GIMP_TYPE_PAINTBRUSH_TOOL        ||
          tool_type == GIMP_TYPE_SMUDGE_TOOL);
}

static gboolean
tool_has_rate_dynamics (GType tool_type)
{
  return (tool_type == GIMP_TYPE_AIRBRUSH_TOOL          ||
          tool_type == GIMP_TYPE_CONVOLVE_TOOL          ||
          tool_type == GIMP_TYPE_SMUDGE_TOOL);
}

static gboolean
tool_has_size_dynamics (GType tool_type)
{
  return (g_type_is_a (tool_type, GIMP_TYPE_PAINTBRUSH_TOOL) ||
          tool_type == GIMP_TYPE_CLONE_TOOL             ||
          tool_type == GIMP_TYPE_HEAL_TOOL              ||
          tool_type == GIMP_TYPE_PERSPECTIVE_CLONE_TOOL ||
          tool_type == GIMP_TYPE_CONVOLVE_TOOL          ||
          tool_type == GIMP_TYPE_DODGE_BURN_TOOL        ||
          tool_type == GIMP_TYPE_ERASER_TOOL);
}

static gboolean
tool_has_aspect_ratio_dynamics (GType tool_type)
{
  return (g_type_is_a (tool_type, GIMP_TYPE_PAINTBRUSH_TOOL) ||
          tool_type == GIMP_TYPE_CLONE_TOOL             ||
          tool_type == GIMP_TYPE_HEAL_TOOL              ||
          tool_type == GIMP_TYPE_PERSPECTIVE_CLONE_TOOL ||
          tool_type == GIMP_TYPE_CONVOLVE_TOOL          ||
          tool_type == GIMP_TYPE_DODGE_BURN_TOOL        ||
          tool_type == GIMP_TYPE_ERASER_TOOL);
}

static gboolean
tool_has_angle_dynamics (GType tool_type)
{
  return (g_type_is_a (tool_type, GIMP_TYPE_PAINTBRUSH_TOOL));
}

static gboolean
tool_has_color_dynamics (GType tool_type)
{
  return (g_type_is_a (tool_type, GIMP_TYPE_PAINTBRUSH_TOOL));
}

static GtkWidget *
dynamics_check_button_new (GObject     *config,
                           const gchar *property_name,
                           GtkTable    *table,
                           gint         column,
                           gint         row)
{
  GtkWidget *button;

  button = gimp_prop_check_button_new (config, property_name, NULL);
  gtk_table_attach (table, button, column, column + 1, row, row + 1,
                    GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_widget_show (button);

  return button;
}

static void
dynamics_check_button_size_allocate (GtkWidget     *toggle,
                                     GtkAllocation *allocation,
                                     GtkWidget     *label)
{
  GtkWidget     *fixed = gtk_widget_get_parent (label);
  GtkAllocation  label_allocation;
  GtkAllocation  fixed_allocation;
  gint           x, y;

  gtk_widget_get_allocation (label, &label_allocation);
  gtk_widget_get_allocation (fixed, &fixed_allocation);

  if (gtk_widget_get_direction (label) == GTK_TEXT_DIR_LTR)
    x = allocation->x;
  else
    x = allocation->x + allocation->width - label_allocation.width;

  x -= fixed_allocation.x;

  y = fixed_allocation.height - label_allocation.height;

  gtk_fixed_move (GTK_FIXED (fixed), label, x, y);
}

static void
pressure_options_gui (GimpPaintOptions *paint_options,
                      GType             tool_type,
                      GtkTable         *table,
                      gint              row,
                      GtkWidget        *labels[])
{
  GObject   *config = G_OBJECT (paint_options);
  GtkWidget *button;
  gint       column = 1;
  GtkWidget *scalebutton;

  if (tool_has_opacity_dynamics (tool_type))
    {
      button = dynamics_check_button_new (config, "pressure-opacity",
                                          table, column, row);
      g_signal_connect (button, "size-allocate",
                        G_CALLBACK (dynamics_check_button_size_allocate),
                        labels[column - 1]);
      column++;
    }

  if (tool_has_hardness_dynamics (tool_type))
    {
      button = dynamics_check_button_new (config, "pressure-hardness",
                                          table, column, row);
      g_signal_connect (button, "size-allocate",
                        G_CALLBACK (dynamics_check_button_size_allocate),
                        labels[column - 1]);
      column++;
    }

  if (tool_has_rate_dynamics (tool_type))
    {
      button = dynamics_check_button_new (config, "pressure-rate",
                                          table, column, row);
      g_signal_connect (button, "size-allocate",
                        G_CALLBACK (dynamics_check_button_size_allocate),
                        labels[column - 1]);
      column++;
    }

  if (tool_has_size_dynamics (tool_type))
    {
      if (tool_type != GIMP_TYPE_AIRBRUSH_TOOL)
        button = dynamics_check_button_new (config, "pressure-size",
                                            table, column, row);
      else
        button = dynamics_check_button_new (config, "pressure-inverse-size",
                                            table, column, row);

      g_signal_connect (button, "size-allocate",
                        G_CALLBACK (dynamics_check_button_size_allocate),
                        labels[column - 1]);
      column++;
    }

  if (tool_has_aspect_ratio_dynamics (tool_type))
    {
      button = dynamics_check_button_new (config, "pressure-aspect_ratio",
                                          table, column, row);

      g_signal_connect (button, "size-allocate",
                        G_CALLBACK (dynamics_check_button_size_allocate),
                        labels[column - 1]);
      column++;
     }

  if (tool_has_angle_dynamics (tool_type))
    {
      button = dynamics_check_button_new (config, "pressure-angle",
                                          table, column, row);
      g_signal_connect (button, "size-allocate",
                        G_CALLBACK (dynamics_check_button_size_allocate),
                        labels[column - 1]);
      column++;
    }

  if (tool_has_color_dynamics (tool_type))
    {
      button = dynamics_check_button_new (config, "pressure-color",
                                          table, column, row);
      g_signal_connect (button, "size-allocate",
                        G_CALLBACK (dynamics_check_button_size_allocate),
                        labels[column - 1]);
      column++;
    }

   scalebutton = gimp_prop_scale_button_new (config, "pressure-prescale");
   gtk_table_attach (table, scalebutton, column, column + 1, row, row + 1,
                    GTK_SHRINK, GTK_SHRINK, 0, 0);
   gtk_widget_show (scalebutton);
}

static void
velocity_options_gui (GimpPaintOptions *paint_options,
                      GType             tool_type,
                      GtkTable         *table,
                      gint              row)
{
  GObject   *config = G_OBJECT (paint_options);
  gint       column = 1;
  GtkWidget *scalebutton;

  if (tool_has_opacity_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "velocity-opacity",
                                 table, column++, row);
    }

  if (tool_has_hardness_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "velocity-hardness",
                                 table, column++, row);
    }

  if (tool_has_rate_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "velocity-rate",
                                 table, column++, row);
    }

  if (tool_has_size_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "velocity-size",
                                 table, column++, row);
    }

  if (tool_has_aspect_ratio_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "velocity-aspect-ratio",
                                 table, column++, row);
    }


  if (tool_has_angle_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "velocity-angle",
                                 table, column++, row);
    }

  if (tool_has_color_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "velocity-color",
                                 table, column++, row);
    }

  scalebutton = gimp_prop_scale_button_new (config, "velocity-prescale");
  gtk_table_attach (table, scalebutton, column, column + 1, row, row + 1,
                    GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_widget_show (scalebutton);
}

static void
direction_options_gui (GimpPaintOptions *paint_options,
                       GType             tool_type,
                       GtkTable         *table,
                       gint              row)
{
  GObject   *config = G_OBJECT (paint_options);
  gint       column = 1;
  GtkWidget *scalebutton;

  if (tool_has_opacity_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "direction-opacity",
                                 table, column++, row);
    }

  if (tool_has_hardness_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "direction-hardness",
                                 table, column++, row);
    }

  if (tool_has_rate_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "direction-rate",
                                 table, column++, row);
    }

  if (tool_has_size_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "direction-size",
                                 table, column++, row);
    }

  if (tool_has_aspect_ratio_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "direction-aspect-ratio",
                                 table, column++, row);
    }

  if (tool_has_angle_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "direction-angle",
                                 table, column++, row);
    }

  if (tool_has_color_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "direction-color",
                                 table, column++, row);
    }

  scalebutton = gimp_prop_scale_button_new (config, "direction-prescale");
  gtk_table_attach (table, scalebutton, column, column + 1, row, row + 1,
                    GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_widget_show (scalebutton);
}


static void
tilt_options_gui (GimpPaintOptions *paint_options,
                       GType             tool_type,
                       GtkTable         *table,
                       gint              row)
{
  GObject   *config = G_OBJECT (paint_options);
  gint       column = 1;
  GtkWidget *scalebutton;

  if (tool_has_opacity_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "tilt-opacity",
                                 table, column++, row);
    }

  if (tool_has_hardness_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "tilt-hardness",
                                 table, column++, row);
    }

  if (tool_has_rate_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "tilt-rate",
                                 table, column++, row);
    }

  if (tool_has_size_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "tilt-size",
                                 table, column++, row);
    }

if (tool_has_aspect_ratio_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "tilt-aspect-ratio",
                                 table, column++, row);
    }

  if (tool_has_angle_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "tilt-angle",
                                 table, column++, row);
    }

  if (tool_has_color_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "tilt-color",
                                 table, column++, row);
    }

  scalebutton = gimp_prop_scale_button_new (config, "tilt-prescale");
  gtk_table_attach (table, scalebutton, column, column + 1, row, row + 1,
                    GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_widget_show (scalebutton);
}

static void
random_options_gui (GimpPaintOptions *paint_options,
                    GType             tool_type,
                    GtkTable         *table,
                    gint              row)
{
  GObject   *config = G_OBJECT (paint_options);
  gint       column = 1;
  GtkWidget *scalebutton;

  if (tool_has_opacity_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "random-opacity",
                                 table, column++, row);
    }

  if (tool_has_hardness_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "random-hardness",
                                 table, column++, row);
    }

  if (tool_has_rate_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "random-rate",
                                 table, column++, row);
    }

  if (tool_has_size_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "random-size",
                                 table, column++, row);
    }

  if (tool_has_aspect_ratio_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "random-aspect-ratio",
                                 table, column++, row);
    }

  if (tool_has_angle_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "random-angle",
                                 table, column++, row);
    }

  if (tool_has_color_dynamics (tool_type))
    {
      dynamics_check_button_new (config, "random-color",
                                 table, column++, row);
    }

   scalebutton = gimp_prop_scale_button_new (config, "random-prescale");
   gtk_table_attach (table, scalebutton, column, column + 1, row, row + 1,
                    GTK_SHRINK, GTK_SHRINK, 0, 0);
   gtk_widget_show (scalebutton);
}

static GtkWidget *
fade_options_gui (GimpPaintOptions *paint_options,
                  GType             tool_type)
{
  GObject   *config = G_OBJECT (paint_options);
  GtkWidget *frame;
  GtkWidget *table;
  GtkWidget *scale;
  GtkWidget *menu;
  GtkWidget *combo;
  GtkWidget *checkbox;

  table = gtk_table_new (3, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);

  frame = gimp_prop_expanding_frame_new (config, "use-fade",
                                         _("Fade out"),
                                         table, NULL);

  /*  the fade-out sizeentry  */
  scale = gimp_prop_spin_scale_new (config, "fade-length",
                                    _("Length"), 1.0, 50.0, 0);
  gtk_table_attach (GTK_TABLE (table), scale, 0, 2, 0, 1,
                    GTK_EXPAND | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (scale);

  /*  the fade-out unitmenu  */
  menu = gimp_prop_unit_combo_box_new (config, "fade-unit");
  gtk_table_attach (GTK_TABLE (table), menu, 2, 3, 0, 1,
                    GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (menu);

#if 0
  /* FIXME pixel digits */
  g_object_set_data (G_OBJECT (menu), "set_digits", spinbutton);
  gimp_unit_menu_set_pixel_digits (GIMP_UNIT_MENU (menu), 0);
#endif

    /*  the repeat type  */
  combo = gimp_prop_enum_combo_box_new (config, "fade-repeat", 0, 0);
  gimp_table_attach_aligned (GTK_TABLE (table), 0, 2,
                             _("Repeat:"), 0.0, 0.5,
                             combo, 2, FALSE);

  checkbox = gimp_prop_check_button_new (config,
                                         "fade-reverse",
                                          _("Reverse"));
  gtk_table_attach (GTK_TABLE (table), checkbox, 0, 2, 3, 4,
                    GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_widget_show (checkbox);

  return frame;
}

static GtkWidget *
jitter_options_gui (GimpPaintOptions *paint_options,
                    GType             tool_type)
{
  GObject   *config = G_OBJECT (paint_options);
  GtkWidget *frame;
  GtkWidget *scale;

  scale = gimp_prop_spin_scale_new (config, "jitter-amount",
                                    _("Amount"),
                                    0.01, 1.0, 2);

  frame = gimp_prop_expanding_frame_new (config, "use-jitter",
                                         _("Apply Jitter"),
                                         scale, NULL);

  return frame;
}

static GtkWidget *
gradient_options_gui (GimpPaintOptions *paint_options,
                      GType             tool_type,
                      GtkWidget        *incremental_toggle)
{
  GObject   *config = G_OBJECT (paint_options);
  GtkWidget *frame;
  GtkWidget *box;
  GtkWidget *button;

  /*  the gradient view  */
  box = gimp_prop_gradient_box_new (NULL, GIMP_CONTEXT (config),
                                    _("Gradient"), 2,
                                    "gradient-view-type",
                                    "gradient-view-size",
                                    "gradient-reverse");

  frame = gimp_prop_expanding_frame_new (config, "use-gradient",
                                         _("Use color from gradient"),
                                         box, &button);

  if (incremental_toggle)
    {
      gtk_widget_set_sensitive (incremental_toggle,
                                ! paint_options->gradient_options->use_gradient);
      g_object_set_data (G_OBJECT (button), "inverse_sensitive",
                         incremental_toggle);
    }

  return frame;
}

static void
gimp_paint_options_gui_reset_size (GtkWidget        *button,
                                   GimpPaintOptions *paint_options)
{
 GimpBrush *brush = gimp_context_get_brush (GIMP_CONTEXT (paint_options));

 if (brush)
   {
     g_object_set (paint_options,
                   "brush-size", (gdouble) MAX (brush->mask->width,
                                                brush->mask->height),
                   NULL);
   }
}
