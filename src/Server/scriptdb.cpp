#include "scriptdb.h"

ScriptDb::ScriptDb(QScriptEngine *engineLink) :
        engine(engineLink), database()
{
}

bool ScriptDb::open(QString const& name)
{
    database = QSqlDatabase::addDatabase("QSQLITE", "scriptconnection");
    database.setDatabaseName(name);
    return database.open();
}

QScriptValue ScriptDb::execute(QScriptContext *context, QScriptEngine *engine)
{
    ScriptDb *script_db = qobject_cast<ScriptDb*>(context->thisObject().toQObject());
    QString const sql = context->argument(0).toString();
    QSqlQuery query(script_db->database);
    if (!query.prepare(sql)) {
        return engine->nullValue();
    }
    for (int i = 1; i < context->argumentCount(); ++i) {
        query.addBindValue(context->argument(i).toVariant());
    }
    if (query.exec()) {
        if (query.isSelect()) {
            QScriptValue record_holder;
            if (query.driver()->hasFeature(QSqlDriver::QuerySize))
                record_holder = engine->newArray(query.size());
            else
                record_holder = engine->newArray();
            int i = 0;
            while (query.next()) {
                QSqlRecord record = query.record();
                QScriptValue value = engine->newObject();
                for (int j = 0; j < record.count(); ++j) {
                    value.setProperty(record.fieldName(j), engine->newVariant(record.value(j)));
                }
                record_holder.setProperty(i++, value);
            }
            return record_holder;
        } else {
            return true;
        }
    } else {
        return engine->nullValue();
    }
}
