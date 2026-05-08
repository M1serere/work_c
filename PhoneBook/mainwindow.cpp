#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QHeaderView>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

// Конструктор окна
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

    QWidget *central = new QWidget();
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout();

    // Поля ввода
    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("Имя");

    phoneEdit = new QLineEdit();
    phoneEdit->setMaxLength(11);
    phoneEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d{0,11}"), phoneEdit));
    phoneEdit->setPlaceholderText("Телефон");

    // Кнопки
    addButton = new QPushButton("Добавить");
    deleteButton = new QPushButton("Удалить");

    // Таблица
    table = new QTableWidget();
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"ID", "Имя", "Телефон"});
    table->horizontalHeader()->setStretchLastSection(true);

    // Layout для ввода
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(nameEdit);
    inputLayout->addWidget(phoneEdit);
    inputLayout->addWidget(addButton);
    inputLayout->addWidget(deleteButton);

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(table);

    central->setLayout(mainLayout);

    // Связь кнопок
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addContact);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteContact);

    loadData();
}

// Загрузка данных
void MainWindow::loadData() {
    table->setRowCount(0);

    QSqlQuery query("SELECT * FROM contacts");

    int row = 0;
    while (query.next()) {
        table->insertRow(row);

        table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));

        row++;
    }
}

// Добавление
void MainWindow::addContact() {
    QString name = nameEdit->text();
    QString phone = phoneEdit->text();

    if (name.isEmpty() || phone.isEmpty()) return;

    if (!QRegularExpression("^\\d{11}$").match(phone).hasMatch()) {
        QMessageBox::warning(this, "Ошибка", "Телефон должен содержать ровно 11 цифр");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO contacts (name, phone) VALUES (:name, :phone)");
    query.bindValue(":name", name);
    query.bindValue(":phone", phone);
    query.exec();

    nameEdit->clear();
    phoneEdit->clear();

    loadData();
}

// Удаление
void MainWindow::deleteContact() {
    int row = table->currentRow();
    if (row < 0) return;

    int id = table->item(row, 0)->text().toInt();

    QSqlQuery query;
    query.prepare("DELETE FROM contacts WHERE id = :id");
    query.bindValue(":id", id);
    query.exec();

    loadData();
}
