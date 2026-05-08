#include "database.h"
#include <QSqlQuery>
#include <QDebug>

void Database::connect() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("phonebook.db");

    if (!db.open()) {
        qDebug() << "Ошибка открытия базы данных";
        return;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS contacts ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "name TEXT,"
               "phone TEXT)");
}