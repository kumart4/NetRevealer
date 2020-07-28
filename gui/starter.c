#include <gtk/gtk.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>

#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */

// static GtkWidget * create_row (const gchar *text){
//   GtkWidget *row, *handle, *box, *label, *image;

//   row = gtk_list_box_row_new ();

//   box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
//   g_object_set (box, "margin-start", 10, "margin-end", 10, NULL);

//   handle = gtk_event_box_new ();
//   gtk_container_add (GTK_CONTAINER (box), handle);

//   label = gtk_label_new (text);
//   gtk_container_add_with_properties (GTK_CONTAINER (box), label, "expand", TRUE, NULL);
//   return row;
// }

enum {
    COL_INDEX,
    COL_TYPE,
    COL_TX_PACKETS,
    COL_RX_PACKETS,
    COL_TX_BYTES,
    COL_RX_BYTES ,
    NUM_COLS
};

gchar *selected_type;
gboolean network_rules_backup = 1;
GObject *gtkWindow;

// enum{
//   COL_NAME = 0,
//   COL_AGE,
//   NUM_COLS
// };

static GtkTreeModel * create_and_fill_model (void){
    GtkListStore  *store;
    GtkTreeIter    iter;
    int i = 0;

    store = gtk_list_store_new (NUM_COLS,G_TYPE_INT, G_TYPE_STRING , G_TYPE_INT, G_TYPE_INT,G_TYPE_INT ,G_TYPE_INT);



    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    /* Walk through linked list, maintaining head pointer so we
        can free list later */
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;
        if (family == AF_PACKET && ifa->ifa_data != NULL) {
            struct rtnl_link_stats *stats = ifa->ifa_data;
            gtk_list_store_append (store, &iter);
            gtk_list_store_set (store, &iter,
                        COL_INDEX, i,
                        COL_TYPE, ifa->ifa_name,
                        COL_TX_PACKETS, stats->tx_packets,
                        COL_RX_PACKETS, stats->rx_packets,
                        COL_TX_BYTES, stats->tx_bytes,
                        COL_RX_BYTES, stats->rx_bytes,
                        -1);
            
        }
        i++;
    }
            

    return GTK_TREE_MODEL (store);
};

static GtkWidget * create_view_and_model (void){
    
    GtkCellRenderer     *renderer;
    GtkTreeModel        *model;
    GtkWidget           *view;

    view = gtk_tree_view_new ();

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                                -1,      
                                                "N°",  
                                                renderer,
                                                "text", COL_INDEX,
                                                NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                                -1,      
                                                "Type",  
                                                renderer,
                                                "text", COL_TYPE,
                                                NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                                -1,      
                                                "TX Packets",  
                                                renderer,
                                                "text", COL_TX_PACKETS,
                                                NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                                -1,      
                                                "RX Packets",  
                                                renderer,
                                                "text", COL_TX_PACKETS,
                                                NULL);
    
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                                -1,      
                                                "TX Bytes",  
                                                renderer,
                                                "text", COL_TX_BYTES,
                                                NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                                -1,      
                                                "RX Bytes",  
                                                renderer,
                                                "text", COL_RX_BYTES,
                                                NULL);

    model = create_and_fill_model ();

    gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

    /* The tree view has acquired its own reference to the
    *  model, so we can drop ours. That way the model will
    *  be freed automatically when the tree view is destroyed */

    g_object_unref (model);

    return view;
};

gboolean view_selection_func (GtkTreeSelection *selection,
                       GtkTreeModel     *model,
                       GtkTreePath      *path,
                       gboolean          path_currently_selected,
                       gpointer          userdata){
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter(model, &iter, path)){
        gchar *type;

        gtk_tree_model_get(model, &iter, COL_TYPE, &type, -1);

        if (!path_currently_selected){
            selected_type = type;
            g_print ("%s is going to be selected.\n", type);
        }
        else{
            g_print ("%s is going to be unselected.\n", type);
        }

    }

    return TRUE; /* allow selection state to change */
}

void gtkWarningContinue_clicked(GtkWidget *widget, gpointer data, GtkWindow *window) {
    GError *error = NULL;
    GtkBuilder *gtkmainWindow;
    GObject *mainWindow;


    
    gtkmainWindow = gtk_builder_new ();
    if (gtk_builder_add_from_file (gtkmainWindow, "mainWindow.ui", &error) == 0){
        g_printerr ("Error loading file: %s\n", error->message);
        g_clear_error (&error);
        return 1;
    }
    mainWindow = gtk_builder_get_object (gtkmainWindow, "mainWindow");
    g_signal_connect (mainWindow,"destroy", G_CALLBACK (gtk_main_quit), NULL);
    gtk_window_close(window);
    gtk_window_close(gtkWindow);
    


}

void gtkWarningCancel_clicked(GtkWidget *widget, gpointer data, GtkWindow *window) {
    
    gtk_window_close(window);
    

}

void gtkToolBarSelect_clicked(GtkWidget *widget, gpointer data) {
    GtkBuilder *warningBuilder;
    GtkWindow *gtkWarningBackup;
    GObject *gtkWarningContinue;
    GObject *gtkWarningCancel;
    GError *error = NULL;

    warningBuilder = gtk_builder_new ();
    if (gtk_builder_add_from_file (warningBuilder, "warning_backup.ui", &error) == 0){
        g_printerr ("Error loading file: %s\n", error->message);
        g_clear_error (&error);
        return 1;
    }

    gtkWarningBackup = gtk_builder_get_object (warningBuilder, "gtkWarningBackup");
    gtkWarningContinue = gtk_builder_get_object (warningBuilder, "gtkWarningContinue");
    gtkWarningCancel = gtk_builder_get_object (warningBuilder, "gtkWarningCancel");
    g_signal_connect (gtkWarningBackup, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (gtkWarningContinue, "clicked", gtkWarningContinue_clicked, gtkWarningBackup);
    g_signal_connect (gtkWarningCancel, "clicked", gtkWarningCancel_clicked, gtkWarningBackup);
    
    gtk_main();
}

void gtkToolBarBackup_clicked(GtkWidget *widget, gpointer data, GtkWindow *window){
    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;
    FILE *fp;
    FILE *output;
    char *save_Backup = "iptables-save";
    char result[8192] = "";
    char buffer[128];



    system("iptables-save > temp.backup;");
    
    dialog = gtk_file_chooser_dialog_new ("Save File",
                                        window,
                                        action,
                                        ("_Cancel"),
                                        GTK_RESPONSE_CANCEL,
                                        ("_Save"),
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);
    chooser = GTK_FILE_CHOOSER (dialog);

    gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

    
    gtk_file_chooser_set_current_name (chooser,
                                         ("Untitled document"));
    
    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT){
        char *filename;

        filename = gtk_file_chooser_get_filename (chooser);
        output = popen(save_Backup,"r");
        if (!output) {
            return "popen failed!";
        }

        // read till end of process:
        while (!feof(output)) {

            // use buffer to read and add to result
            if (fgets(buffer, 128, output) != NULL){
                strcat(result, buffer);
            }
        }
        fclose(output);
        // printf("%s", result);
        fp = fopen(filename, "w");
        fprintf(fp, result);
        fclose(fp);
        g_free (filename);
    }

    gtk_widget_destroy (dialog);
}

int main (int   argc, char *argv[]){
    gchar *text;
    gint i;
    GtkWidget *row;
    GtkBuilder *builder;
    GObject *gtkToolbar;
    GObject *gtkToolBarSelect;
    GObject *gtkToolBarBackup;
    GObject *gtkListBox;
    GObject *gtkLabelLogo;
    GObject *gtkGrid;

    GError *error = NULL;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkWidget *view;
    gtk_init (&argc, &argv);

    /* Construct a GtkBuilder instance and load our UI description */
    builder = gtk_builder_new ();
    if (gtk_builder_add_from_file (builder, "starter.ui", &error) == 0){
        g_printerr ("Error loading file: %s\n", error->message);
        g_clear_error (&error);
        return 1;
    }

    /* Connect signal handlers to the constructed widgets. */
    gtkWindow = gtk_builder_get_object (builder, "starter");
    gtkGrid =  gtk_builder_get_object(builder, "gtkGrid");
    gtkToolBarSelect = gtk_builder_get_object(builder, "gtkToolBarSelect");
    gtkToolBarBackup = gtk_builder_get_object(builder, "gtkToolBarBackup");
    view = create_view_and_model ();
    // gtk_container_add (GTK_CONTAINER (gtkGrid), view);
    gtk_grid_attach ((GtkGrid *)gtkGrid, view, 0, 3, 2, 1);

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

    gtk_tree_selection_set_select_function(selection, view_selection_func, NULL, NULL);
    
    g_signal_connect (gtkWindow, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (gtkToolBarSelect, "clicked", gtkToolBarSelect_clicked, NULL);
    g_signal_connect (gtkToolBarBackup, "clicked", gtkToolBarBackup_clicked, gtkWindow);
    
    gtk_widget_show_all (gtkWindow);


    

    gtk_main ();
    
    return 0;
}