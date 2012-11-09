#include "Gui.hxx"
#include "DebugTools.h"

#include "ScriptingConsole.hxx"

ServerGui::ServerGui( QWidget* parent, Qt::WFlags flags )
    : QMainWindow( parent, flags )
{
    ui.setupUi( this );

    QCoreApplication::setOrganizationName("VT");
    QCoreApplication::setApplicationName("VTScripter");

    ui.debug_group_box->setVisible(true);
    ui.edit_debug_output->setMaximumBlockCount(DebugOutput::output_line_limit);
    DebugOutput::sink = ui.edit_debug_output;
    qInstallMsgHandler(DebugOutput::debugMessageDisplayFunc);

    set_up_debug_widget();

    connect( ui.actionScripting_console, SIGNAL( triggered() ), this, SLOT( open_scripting_console() ) );
    connect( ui.actionDebug_Console, SIGNAL( toggled(bool) ), ui.debug_group_box, SLOT( setVisible(bool) ) );
}

ServerGui::~ServerGui()
{
}

void ServerGui::closeEvent(QCloseEvent *event)
{
    qApp->closeAllWindows();
}

void ServerGui::open_scripting_console()
{
    ScriptingConsole* sc = new ScriptingConsole();
    sc->setAttribute(Qt::WA_DeleteOnClose);
    sc->show();
}

void ServerGui::menu_test1()
{
}

void ServerGui::menu_test2()
{
}

void ServerGui::set_up_debug_widget()
{
    ui.table_var_debug->setColumnCount(4);
    QStringList headers;
    headers << "Name" << "Value" << "Type" << "Is Global";
    ui.table_var_debug->setHorizontalHeaderLabels(headers);
    ui.table_var_debug->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    ui.table_var_debug->horizontalHeader()->setHighlightSections(false);

    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_debug_widget()));
    timer->start(100);
}

void ServerGui::update_debug_widget()
{
    // Useful comment:
    // Fuck selection! I'm setting the table to non-selectable and non-focusable
    /*
    QString selected_name;
    QTableWidgetItem* selected_item = ui.table_var_debug->item(ui.table_var_debug->currentRow(), 0);
    if (selected_item)
        selected_name = selected_item->text();      // get name of selected variable
    int selected_column = ui.table_var_debug->currentColumn();
    */
    /*
    //ui.table_var_debug->setCurrentCell(-1, -1);
    //ui.table_var_debug->clearSelection();
    ui.table_var_debug->setSortingEnabled(false);

    const Lua::VarMap& variables = lua_interpreter_.variables();
    ui.table_var_debug->setRowCount(variables.size());
    int i = 0;
    for (Lua::VarMap::const_iterator it = variables.begin(); it != variables.end(); ++it)
    {
        const Lua::Variable& variable = it->second;

        QTableWidgetItem* name = new QTableWidgetItem( QString::fromStdString(variable.name_) );
        QTableWidgetItem* value = new QTableWidgetItem( QString::fromStdString(variable.value_) );
        QTableWidgetItem* type = new QTableWidgetItem( QString::fromStdString(variable.type_) );
        QTableWidgetItem* is_global = new QTableWidgetItem( variable.is_global_ ? "True" : "False" );

        ui.table_var_debug->setItem(i, 0, name);
        ui.table_var_debug->setItem(i, 1, value);
        ui.table_var_debug->setItem(i, 2, type);
        ui.table_var_debug->setItem(i, 3, is_global);*/
/*
        if ( selected_name == variable.name_ )
            ui.table_var_debug->selectRow(i);
*/
 /*       ++i;
    }

    ui.table_var_debug->setSortingEnabled(true);
    ui.table_var_debug->sortItems( ui.table_var_debug->horizontalHeader()->sortIndicatorSection(), 
                                   ui.table_var_debug->horizontalHeader()->sortIndicatorOrder() );*/
}
