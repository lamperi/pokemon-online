#ifndef SCRIPTDB_H
#define SCRIPTDB_H

#include <QObject>
#include <QtScript>
#include <QString>
#include <QtSql>


class ScriptDb : public QObject
{
Q_OBJECT
public:
    explicit ScriptDb(QScriptEngine *engineLink);

    Q_INVOKABLE bool open(QString const & database);

    static QScriptValue execute(QScriptContext *context, QScriptEngine *);

private:
    QScriptEngine *engine;
    QSqlDatabase database;

};


#endif // SCRIPTDB_H
