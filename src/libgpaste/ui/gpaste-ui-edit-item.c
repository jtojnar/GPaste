/*
 *      This file is part of GPaste.
 *
 *      Copyright 2015 Marc-Antoine Perennou <Marc-Antoine@Perennou.com>
 *
 *      GPaste is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      GPaste is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with GPaste.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gpaste-ui-edit-item.h>

struct _GPasteUiEditItem
{
    GtkButton parent_instance;
};

typedef struct
{
    GtkWindow    *rootwin;
} GPasteUiEditItemPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GPasteUiEditItem, g_paste_ui_edit_item, G_PASTE_TYPE_UI_ITEM_ACTION)

typedef struct
{
    GPasteUiEditItemPrivate *priv;
    guint32                  index;
} CallbackData;

static void
on_item_ready (GObject      *source_object,
               GAsyncResult *res,
               gpointer      user_data)
{
    g_autofree CallbackData *data = user_data;
    GPasteUiEditItemPrivate *priv = data->priv;
    GPasteClient *client = G_PASTE_CLIENT (source_object);
    g_autofree gchar *old_txt = g_paste_client_get_raw_element_finish (client, res, NULL);

    if (!old_txt)
        return;

    GtkWidget *dialog = gtk_dialog_new_with_buttons ("GPaste", priv->rootwin,
                                                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_USE_HEADER_BAR,
                                                     _("Edit"),   GTK_RESPONSE_OK,
                                                     _("Cancel"), GTK_RESPONSE_CANCEL,
                                                     NULL);
    GtkDialog *d = GTK_DIALOG (dialog);
    GtkWidget *text = gtk_text_view_new ();
    GtkTextView *tv = GTK_TEXT_VIEW (text);
    GtkTextBuffer *buf = gtk_text_view_get_buffer (tv);
    GtkWidget *scroll = gtk_scrolled_window_new (NULL, NULL);
    GtkScrolledWindow *sw = GTK_SCROLLED_WINDOW (scroll);

    gtk_text_view_set_wrap_mode (tv, GTK_WRAP_WORD);
    gtk_text_buffer_set_text (buf, old_txt, -1);
    gtk_scrolled_window_set_min_content_height (sw, 300);
    gtk_scrolled_window_set_min_content_width (sw, 600);
    gtk_container_add (GTK_CONTAINER (sw), text);
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (d)), scroll, TRUE, TRUE, 0);
    gtk_widget_show_all (scroll);

    if (gtk_dialog_run (d) == GTK_RESPONSE_OK)
    {
        const gchar *txt;

        g_object_get (G_OBJECT (buf), "text", &txt, NULL);
        if (txt && *txt)
            g_paste_client_replace (client, data->index, txt, NULL, NULL);
    }

    gtk_widget_destroy (dialog);
}

static void
g_paste_ui_edit_item_activate (GPasteUiItemAction *self,
                               GPasteClient       *client,
                               guint32             index)
{
    CallbackData *data = g_malloc (sizeof (CallbackData));

    data->priv = g_paste_ui_edit_item_get_instance_private (G_PASTE_UI_EDIT_ITEM (self));
    data->index = index;

    g_paste_client_get_raw_element (client, index, on_item_ready, data);
}

static void
g_paste_ui_edit_item_class_init (GPasteUiEditItemClass *klass)
{
    G_PASTE_UI_ITEM_ACTION_CLASS (klass)->activate = g_paste_ui_edit_item_activate;
}

static void
g_paste_ui_edit_item_init (GPasteUiEditItem *self G_GNUC_UNUSED)
{
}

/**
 * g_paste_ui_edit_item_new:
 * @client: a #GPasteClient
 * @rootwin: the root #GtkWindow
 *
 * Create a new instance of #GPasteUiEditItem
 *
 * Returns: a newly allocated #GPasteUiEditItem
 *          free it with g_object_unref
 */
G_PASTE_VISIBLE GtkWidget *
g_paste_ui_edit_item_new (GPasteClient *client,
                          GtkWindow    *rootwin)
{
    g_return_val_if_fail (G_PASTE_IS_CLIENT (client), NULL);
    g_return_val_if_fail (GTK_IS_WINDOW (rootwin), NULL);

    GtkWidget *self = g_paste_ui_item_action_new (G_PASTE_TYPE_UI_EDIT_ITEM, client, "accessories-text-editor-symbolic", _("Edit"));
    GPasteUiEditItemPrivate *priv = g_paste_ui_edit_item_get_instance_private (G_PASTE_UI_EDIT_ITEM (self));

    priv->rootwin = rootwin;

    return self;
}