/*
 * Student Result Management — GTK 3 graphical interface for updated_main.c.
 * IMPORTANT: The user's provided updated_main.c is included UNMODIFIED.
 * The GUI calls its calculateResult, loadFromFile, saveToFile, loadSampleData,
 * exportReport, findStudent etc. Original 3-line students.txt format is kept.
 *
 * Run on macOS: ./run_mac.sh
 */
#include <gtk/gtk.h>
#include <errno.h>
#include <math.h>
/* The terminal main is renamed, while ALL data functions remain untouched. */
#define main student_terminal_main
#include "updated_main.c"
#undef main

static GtkWidget *window;
static GtkWidget *students_tree;
static GtkWidget *search_entry;
static GtkWidget *stats_label;
static GtkListStore *students_store;
static GtkListStore *rank_store;
static GtkListStore *subject_store;

enum { ST_ROLL, ST_NAME, ST_1, ST_2, ST_3, ST_4, ST_5,
       ST_TOTAL, ST_PERCENT, ST_BACKLOGS, ST_GRADE, ST_STATUS, ST_INDEX, ST_COUNT };
enum { RK_RANK, RK_ROLL, RK_NAME, RK_TOTAL, RK_PERCENT, RK_BACKLOGS, RK_STATUS, RK_COUNT };
enum { SJ_SUBJECT, SJ_AVERAGE, SJ_TOPPER, SJ_MARK, SJ_FAILED, SJ_COUNT };

static void notice(GtkMessageType kind, const char *message) {
    GtkWidget *dlg = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL,
                                             kind, GTK_BUTTONS_OK, "%s", message);
    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
}

static void save_or_warn(void) {
    /* Preserve the original program's three-line file format and save routine. */
    FILE *check = fopen("students.txt", "a");
    if (!check) {
        notice(GTK_MESSAGE_ERROR,
               "Cannot write students.txt in this folder. Check permissions.\n"
               "Changes are visible now but could not be saved.");
        return;
    }
    fclose(check);
    saveToFile();
}

static void refresh_students(void) {
    gtk_list_store_clear(students_store);
    const char *query = gtk_entry_get_text(GTK_ENTRY(search_entry));
    char *needle = g_utf8_casefold(query, -1);
    for (int i = 0; i < count; ++i) {
        student_details *s = &list[i];
        char roll[32], total[32], percentage[32];
        snprintf(roll, sizeof roll, "%d", s->roll_number);
        char *folded_name = g_utf8_casefold(s->name, -1);
        int match = !needle[0] || strstr(roll, query) || strstr(folded_name, needle);
        g_free(folded_name);
        if (!match) continue;
        snprintf(total, sizeof total, "%.2f / 500", s->total);
        snprintf(percentage, sizeof percentage, "%.2f%%", s->percentage);
        char mark_values[SUBJECTS][32];
        for (int j = 0; j < SUBJECTS; ++j)
            snprintf(mark_values[j], sizeof mark_values[j], "%.2f%s",
                     s->marks[j], s->marks[j] < PASS_MARK ? "  BACKLOG" : "");
        GtkTreeIter row;
        gtk_list_store_append(students_store, &row);
        gtk_list_store_set(students_store, &row,
            ST_ROLL, s->roll_number, ST_NAME, s->name,
            ST_1, mark_values[0], ST_2, mark_values[1], ST_3, mark_values[2],
            ST_4, mark_values[3], ST_5, mark_values[4],
            ST_TOTAL, total, ST_PERCENT, percentage, ST_BACKLOGS, s->backlogs,
            ST_GRADE, s->grade, ST_STATUS, s->status,
            ST_INDEX, i, -1);
    }
    g_free(needle);
}

static void ranked_indices(int order[MAX]) {
    for (int i = 0; i < count; ++i) order[i] = i;
    /* Exactly as updated_main.c: passing students first; within each group,
       rank by total marks descending. Keep input order for ties. */
    for (int i = 0; i < count - 1; ++i) {
        for (int j = 0; j < count - i - 1; ++j) {
            const student_details *a = &list[order[j]];
            const student_details *b = &list[order[j+1]];
            int a_failed = a->backlogs > 0, b_failed = b->backlogs > 0;
            if (a_failed > b_failed ||
                (a_failed == b_failed && a->total < b->total)) {
                int tmp = order[j]; order[j] = order[j+1]; order[j+1] = tmp;
            }
        }
    }
}

static void refresh_ranks(void) {
    gtk_list_store_clear(rank_store);
    int order[MAX];
    ranked_indices(order);
    int rank = 1;
    for (int k = 0; k < count; ++k) {
        student_details *s = &list[order[k]];
        if (k > 0) {
            const student_details *p = &list[order[k-1]];
            if (s->total != p->total ||
                (s->backlogs > 0) != (p->backlogs > 0)) rank = k + 1;
        }
        char total[32], percentage[32];
        snprintf(total, sizeof total, "%.2f", s->total);
        snprintf(percentage, sizeof percentage, "%.2f%%", s->percentage);
        GtkTreeIter row;
        gtk_list_store_append(rank_store, &row);
        gtk_list_store_set(rank_store, &row,
            RK_RANK, rank, RK_ROLL, s->roll_number, RK_NAME, s->name,
            RK_TOTAL, total, RK_PERCENT, percentage, RK_BACKLOGS, s->backlogs,
            RK_STATUS, s->status, -1);
    }
}

static void refresh_statistics(void) {
    gtk_list_store_clear(subject_store);
    if (count == 0) {
        gtk_label_set_text(GTK_LABEL(stats_label),
            "No students yet. Add a student or load the sample data.");
        return;
    }
    int passed = 0, topper = 0, total_backlogs = 0;
    double class_total = 0;
    for (int i = 0; i < count; ++i) {
        class_total += list[i].percentage;
        total_backlogs += list[i].backlogs;
        if (strcmp(list[i].status, "PASS") == 0) ++passed;
        /* Original showStatistics() selects max total, regardless of pass/fail */
        if (list[i].total > list[topper].total) topper = i;
    }
    char *text = g_strdup_printf(
        "TOTAL STUDENTS   %d          PASSED   %d          FAILED   %d          BACKLOGS   %d\n"
        "PASS RATE   %.2f%%          CLASS AVERAGE   %.2f%%\n"
        "Overall topper by total: %s  (%.2f / 500)",
        count, passed, count - passed, total_backlogs,
        passed * 100.0 / count, class_total / count,
        list[topper].name, list[topper].total);
    gtk_label_set_text(GTK_LABEL(stats_label), text);
    g_free(text);
    for (int j = 0; j < SUBJECTS; ++j) {
        double sum = 0;
        int subject_topper = 0, failed = 0;
        for (int i = 0; i < count; ++i) {
            sum += list[i].marks[j];
            if (list[i].marks[j] > list[subject_topper].marks[j])
                subject_topper = i;
            if (list[i].marks[j] < PASS_MARK) failed++;
        }
        char avg[32], top_mark[32], label[64];
        snprintf(label, sizeof label, "%d. %s", j+1, subjects[j]);
        snprintf(avg, sizeof avg, "%.2f", sum / count);
        snprintf(top_mark, sizeof top_mark, "%.2f", list[subject_topper].marks[j]);
        GtkTreeIter row;
        gtk_list_store_append(subject_store, &row);
        gtk_list_store_set(subject_store, &row,
            SJ_SUBJECT, label, SJ_AVERAGE, avg,
            SJ_TOPPER, list[subject_topper].name,
            SJ_MARK, top_mark, SJ_FAILED, failed, -1);
    }
}

static void refresh_all(void) {
    refresh_students();
    refresh_ranks();
    refresh_statistics();
}

static int selected_student_index(void) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(students_tree));
    GtkTreeModel *model;
    GtkTreeIter iter;
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) return -1;
    int index = -1;
    gtk_tree_model_get(model, &iter, ST_INDEX, &index, -1);
    return index;
}

static int read_roll(const char *text, int *out) {
    if (!*text) return 0;
    for (const char *p = text; *p; ++p)
        if (*p < '0' || *p > '9') return 0;
    errno = 0;
    char *end;
    long roll = strtol(text, &end, 10);
    if (errno || *end || roll < 1 || roll > 999999999L) return 0;
    *out = (int)roll;
    return 1;
}

/* idx == -1: add; otherwise edit while retaining the existing roll number. */
static int student_form(int idx) {
    const int editing = idx >= 0;
    GtkWidget *dlg = gtk_dialog_new_with_buttons(
        editing ? "Edit Student" : "Add Student", GTK_WINDOW(window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_OK, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dlg), GTK_RESPONSE_OK);
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 22);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dlg))), grid);

    GtkWidget *roll_entry = gtk_entry_new();
    GtkWidget *name_entry = gtk_entry_new();
    gtk_widget_set_size_request(name_entry, 300, -1);
    gtk_entry_set_max_length(GTK_ENTRY(name_entry), 99);
    GtkWidget *roll_label = gtk_label_new("Roll number");
    GtkWidget *name_label = gtk_label_new("Student name");
    gtk_widget_set_halign(roll_label, GTK_ALIGN_START);
    gtk_widget_set_halign(name_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), roll_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), roll_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 1, 1, 1);

    GtkWidget *mark_inputs[SUBJECTS];
    for (int j = 0; j < SUBJECTS; ++j) {
        char title[90];
        snprintf(title, sizeof title, "%d. %s (min %d)", j + 1, subjects[j], PASS_MARK);
        GtkWidget *label = gtk_label_new(title);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        mark_inputs[j] = gtk_spin_button_new_with_range(0.0, 100.0, 0.5);
        gtk_spin_button_set_digits(GTK_SPIN_BUTTON(mark_inputs[j]), 2);
        gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(mark_inputs[j]), TRUE);
        gtk_grid_attach(GTK_GRID(grid), label, 0, j + 2, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), mark_inputs[j], 1, j + 2, 1, 1);
    }
    if (editing) {
        char roll_str[32];
        snprintf(roll_str, sizeof roll_str, "%d", list[idx].roll_number);
        gtk_entry_set_text(GTK_ENTRY(roll_entry), roll_str);
        gtk_widget_set_sensitive(roll_entry, FALSE);
        gtk_entry_set_text(GTK_ENTRY(name_entry), list[idx].name);
        for (int j = 0; j < SUBJECTS; ++j)
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(mark_inputs[j]), list[idx].marks[j]);
    }
    gtk_widget_show_all(dlg);
    int saved = 0;
    while (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        int roll = 0;
        if (!read_roll(gtk_entry_get_text(GTK_ENTRY(roll_entry)), &roll)) {
            notice(GTK_MESSAGE_WARNING, "Roll number must be between 1 and 999999999.");
            continue;
        }
        char *name = g_strdup(gtk_entry_get_text(GTK_ENTRY(name_entry)));
        g_strstrip(name);
        if (!name[0] || strlen(name) >= sizeof(((student_details *)0)->name) ||
            strchr(name, '|') || strchr(name, '\t')) {
            g_free(name);
            notice(GTK_MESSAGE_WARNING,
                   "Name must be 1-99 bytes and cannot contain '|' or a tab.");
            continue;
        }
        if ((!editing && findStudent(roll) != -1) ||
            (editing && findStudent(roll) != idx)) {
            g_free(name);
            notice(GTK_MESSAGE_WARNING, "That roll number already exists.");
            continue;
        }
        student_details s = {0};
        s.roll_number = roll;
        snprintf(s.name, sizeof s.name, "%s", name);
        g_free(name);
        for (int j = 0; j < SUBJECTS; ++j) {
            gtk_spin_button_update(GTK_SPIN_BUTTON(mark_inputs[j]));
            s.marks[j] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(mark_inputs[j]));
        }
        if (editing) {
            list[idx] = s;
            calculateResult(idx);
        } else {
            list[count] = s;
            calculateResult(count);
            ++count;
        }
        save_or_warn();
        refresh_all();
        saved = 1;
        break;
    }
    gtk_widget_destroy(dlg);
    return saved;
}

static void on_add(GtkButton *button, gpointer user_data) {
    (void)button; (void)user_data;
    if (count >= MAX) {
        notice(GTK_MESSAGE_WARNING, "Maximum of 100 students reached.");
        return;
    }
    student_form(-1);
}

static void on_edit(GtkButton *button, gpointer user_data) {
    (void)button; (void)user_data;
    int idx = selected_student_index();
    if (idx < 0) {
        notice(GTK_MESSAGE_INFO, "Select a student from the Students table first.");
        return;
    }
    student_form(idx);
}

static void on_delete(GtkButton *button, gpointer user_data) {
    (void)button; (void)user_data;
    int idx = selected_student_index();
    if (idx < 0) {
        notice(GTK_MESSAGE_INFO, "Select a student from the Students table first.");
        return;
    }
    GtkWidget *dlg = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        "Delete %s (roll %d)?", list[idx].name, list[idx].roll_number);
    int answer = gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
    if (answer != GTK_RESPONSE_YES) return;
    for (int i = idx; i < count - 1; ++i) list[i] = list[i + 1];
    --count;
    save_or_warn();
    refresh_all();
}

static void show_student_details(int idx) {
    if (idx < 0) return;
    student_details *s = &list[idx];
    GString *details = g_string_new(NULL);
    g_string_append_printf(details, "Roll number: %d\nName: %s\n\n", s->roll_number, s->name);
    for (int j = 0; j < SUBJECTS; ++j)
        g_string_append_printf(details, "%d. %s: %.2f / 100  [%s]\n",
                               j + 1, subjects[j], s->marks[j],
                               s->marks[j] >= PASS_MARK ? "PASS" : "FAIL");
    g_string_append_printf(details, "\nMinimum required: %d in EVERY subject.\nTotal: %.2f / 500\nPercentage: %.2f%%\nBacklogs: %d\nGrade: %s\nStatus: %s",
        PASS_MARK, s->total, s->percentage, s->backlogs, s->grade, s->status);
    notice(GTK_MESSAGE_INFO, details->str);
    g_string_free(details, TRUE);
}

static void on_details(GtkButton *button, gpointer user_data) {
    (void)button; (void)user_data;
    int idx = selected_student_index();
    if (idx < 0) notice(GTK_MESSAGE_INFO, "Select a student to see subject marks and result.");
    else show_student_details(idx);
}

static void on_row_activated(GtkTreeView *tree, GtkTreePath *path,
                             GtkTreeViewColumn *column, gpointer user_data) {
    (void)tree; (void)path; (void)column; (void)user_data;
    show_student_details(selected_student_index());
}

static void on_sample(GtkButton *button, gpointer user_data) {
    (void)button; (void)user_data;
    int previous = count;
    loadSampleData(); /* Original routine checks duplicate rolls and saves. */
    refresh_all();
    char message[120];
    snprintf(message, sizeof message,
        "%d sample students added. Existing roll numbers were skipped.", count - previous);
    notice(GTK_MESSAGE_INFO, message);
}

static void on_export(GtkButton *button, gpointer user_data) {
    (void)button; (void)user_data;
    if (count == 0) {
        notice(GTK_MESSAGE_INFO, "No students to export.");
        return;
    }
    /* Keep the source code's report.csv field names and quoting. */
    FILE *check = fopen("report.csv", "a");
    if (!check) {
        notice(GTK_MESSAGE_ERROR,
               "Could not write report.csv in this folder. Check permissions.");
        return;
    }
    fclose(check);
    exportReport();
    char message[150];
    snprintf(message, sizeof message,
             "Exported the report for %d students to report.csv in this project folder.", count);
    notice(GTK_MESSAGE_INFO, message);
}

static void on_search(GtkEditable *editable, gpointer user_data) {
    (void)editable; (void)user_data;
    refresh_students();
}

static void add_column(GtkWidget *tree, const char *title, int field, int expand) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "ypad", 7, NULL);
    GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
        title, renderer, "text", field, NULL);
    gtk_tree_view_column_set_resizable(col, TRUE);
    gtk_tree_view_column_set_expand(col, expand);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);
}

static GtkWidget *scrolled_table(GtkWidget *tree) {
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), tree);
    gtk_widget_set_vexpand(scroll, TRUE);
    return scroll;
}

static GtkWidget *action_button(const char *label, GCallback callback) {
    GtkWidget *button = gtk_button_new_with_label(label);
    g_signal_connect(button, "clicked", callback, NULL);
    return button;
}

static void create_ui(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Student Result Management System");
    gtk_window_set_default_size(GTK_WINDOW(window), 1420, 760);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_container_set_border_width(GTK_CONTAINER(root), 20);
    gtk_container_add(GTK_CONTAINER(window), root);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='xx-large' weight='bold'>Student Result Management</span>");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(root), title, FALSE, FALSE, 0);
    GtkWidget *subtitle = gtk_label_new("All five subjects must score at least 35/100. Any lower mark adds a backlog and makes the overall result FAIL.");
    gtk_widget_set_halign(subtitle, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(root), subtitle, FALSE, FALSE, 0);

    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(root), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), action_button("+ Add Student", G_CALLBACK(on_add)), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), action_button("Edit", G_CALLBACK(on_edit)), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), action_button("Delete", G_CALLBACK(on_delete)), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), action_button("View Details", G_CALLBACK(on_details)), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), action_button("Load Sample Data", G_CALLBACK(on_sample)), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), action_button("Export CSV", G_CALLBACK(on_export)), FALSE, FALSE, 0);
    GtkWidget *spacer = gtk_label_new("");
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_pack_start(GTK_BOX(toolbar), spacer, TRUE, TRUE, 0);
    search_entry = gtk_search_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Search name or roll");
    gtk_widget_set_size_request(search_entry, 230, -1);
    g_signal_connect(search_entry, "changed", G_CALLBACK(on_search), NULL);
    gtk_box_pack_end(GTK_BOX(toolbar), search_entry, FALSE, FALSE, 0);

    GtkWidget *notebook = gtk_notebook_new();
    gtk_widget_set_vexpand(notebook, TRUE);
    gtk_box_pack_start(GTK_BOX(root), notebook, TRUE, TRUE, 0);

    students_store = gtk_list_store_new(ST_COUNT,
        G_TYPE_INT, G_TYPE_STRING,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_INT);
    students_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(students_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(students_tree), TRUE);
    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(students_tree), TRUE);
    add_column(students_tree, "Roll", ST_ROLL, FALSE);
    add_column(students_tree, "Name", ST_NAME, TRUE);
    add_column(students_tree, "1. AEM I", ST_1, FALSE);
    add_column(students_tree, "2. Physics", ST_2, FALSE);
    add_column(students_tree, "3. Programming in C", ST_3, FALSE);
    add_column(students_tree, "4. Problem Solving", ST_4, FALSE);
    add_column(students_tree, "5. Environmental Studies", ST_5, FALSE);
    add_column(students_tree, "Total", ST_TOTAL, FALSE);
    add_column(students_tree, "Percentage", ST_PERCENT, FALSE);
    add_column(students_tree, "Backlogs", ST_BACKLOGS, FALSE);
    add_column(students_tree, "Grade", ST_GRADE, FALSE);
    add_column(students_tree, "Status", ST_STATUS, FALSE);
    g_signal_connect(students_tree, "row-activated", G_CALLBACK(on_row_activated), NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_table(students_tree),
                              gtk_label_new("Students"));

    rank_store = gtk_list_store_new(RK_COUNT, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING,
                                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
    GtkWidget *rank_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(rank_store));
    add_column(rank_tree, "Rank", RK_RANK, FALSE);
    add_column(rank_tree, "Roll", RK_ROLL, FALSE);
    add_column(rank_tree, "Name", RK_NAME, TRUE);
    add_column(rank_tree, "Total", RK_TOTAL, FALSE);
    add_column(rank_tree, "Percentage", RK_PERCENT, FALSE);
    add_column(rank_tree, "Backlogs", RK_BACKLOGS, FALSE);
    add_column(rank_tree, "Status", RK_STATUS, FALSE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_table(rank_tree),
                              gtk_label_new("Rank List"));

    GtkWidget *stats_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_set_border_width(GTK_CONTAINER(stats_box), 12);
    stats_label = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(stats_label), 0.0f);
    gtk_label_set_yalign(GTK_LABEL(stats_label), 0.0f);
    gtk_box_pack_start(GTK_BOX(stats_box), stats_label, FALSE, FALSE, 0);
    GtkWidget *stats_heading = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(stats_heading), "<b>Subject-wise Performance</b>");
    gtk_widget_set_halign(stats_heading, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(stats_box), stats_heading, FALSE, FALSE, 0);
    subject_store = gtk_list_store_new(SJ_COUNT, G_TYPE_STRING, G_TYPE_STRING,
                                      G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
    GtkWidget *subject_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(subject_store));
    add_column(subject_tree, "Subject", SJ_SUBJECT, TRUE);
    add_column(subject_tree, "Average Marks", SJ_AVERAGE, FALSE);
    add_column(subject_tree, "Subject Topper", SJ_TOPPER, TRUE);
    add_column(subject_tree, "Top Marks", SJ_MARK, FALSE);
    add_column(subject_tree, "Failed (marks <35)", SJ_FAILED, FALSE);
    gtk_box_pack_start(GTK_BOX(stats_box), scrolled_table(subject_tree), TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), stats_box,
                              gtk_label_new("Statistics"));

    GtkWidget *footer = gtk_label_new("Data: students.txt (original format)  |  CSV export: report.csv  |  Max 100 students  |  Passing mark: 35 in EACH subject");
    gtk_widget_set_halign(footer, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(root), footer, FALSE, FALSE, 0);

    refresh_all();
    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    loadFromFile();
    GtkApplication *app = gtk_application_new("com.example.studentresults", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(create_ui), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
