
#include "scripterrortable.hpp"

#include <QHeaderView>

#include <components/compiler/tokenloc.hpp>
#include <components/compiler/scanner.hpp>
#include <components/compiler/fileparser.hpp>
#include <components/compiler/exception.hpp>
#include <components/compiler/extensions0.hpp>

#include "../../model/doc/document.hpp"


void CSVWorld::ScriptErrorTable::report (const std::string& message, const Compiler::TokenLoc& loc, Type type)
{
    addMessage (message, type==Compiler::ErrorHandler::WarningMessage ?
        CSMDoc::Message::Severity_Warning : CSMDoc::Message::Severity_Error, loc.mLine);
}

void CSVWorld::ScriptErrorTable::report (const std::string& message, Type type)
{
    addMessage (message, type==Compiler::ErrorHandler::WarningMessage ?
        CSMDoc::Message::Severity_Warning : CSMDoc::Message::Severity_Error);
}

void CSVWorld::ScriptErrorTable::addMessage (const std::string& message,
    CSMDoc::Message::Severity severity, int line)
{
    int row = rowCount();

    setRowCount (row+1);

    setItem (row, 0, new QTableWidgetItem (""));

    if (line!=-1)
    {
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setData (Qt::DisplayRole, line+1);
        setItem (row, 1, item);
    }

    setItem (row, 2, new QTableWidgetItem (QString::fromUtf8 (message.c_str())));
}

CSVWorld::ScriptErrorTable::ScriptErrorTable (const CSMDoc::Document& document, QWidget *parent)
: QTableWidget (parent), mContext (document.getData())
{
    setColumnCount (3);

    QStringList headers;
    headers << "Severity" << "Line" << "Description";
    setHorizontalHeaderLabels (headers);
    horizontalHeader()->setStretchLastSection (true);

    Compiler::registerExtensions (mExtensions);
    mContext.setExtensions (&mExtensions);
}

void CSVWorld::ScriptErrorTable::updateUserSetting (const QString& name, const QStringList& value)
{

}

void CSVWorld::ScriptErrorTable::update (const std::string& source)
{
    setRowCount (0);

    try
    {
        std::istringstream input (source);

        Compiler::Scanner scanner (*this, input, mContext.getExtensions());

        Compiler::FileParser parser (*this, mContext);

        scanner.scan (parser);
    }
    catch (const Compiler::SourceException&)
    {
        // error has already been reported via error handler
    }
    catch (const std::exception& error)
    {
        addMessage (error.what(), CSMDoc::Message::Severity_SeriousError);
    }
}
