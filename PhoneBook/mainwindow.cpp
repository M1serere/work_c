#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QHeaderView>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

namespace {
QString normalizeNameInput(const QString &input) {
    QString result;
    int lettersCount = 0;

    for (const QChar &ch : input) {
        if (ch == QLatin1Char(' ')) {
            result.append(ch);
            continue;
        }

        if (!ch.isLetter() || lettersCount >= 10) {
            continue;
        }

        result.append(ch);
        lettersCount++;
    }

    return result;
}

int countLetters(const QString &input) {
    int lettersCount = 0;

    for (const QChar &ch : input) {
        if (ch.isLetter()) {
            lettersCount++;
        }
    }

    return lettersCount;
}
}

// Конструктор окна
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

    QWidget *central = new QWidget();
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout();

    // Поля ввода
    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("Имя");

    connect(nameEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        QString normalized = normalizeNameInput(text);
        if (normalized == text) {
            return;
        }

        int cursorPosition = nameEdit->cursorPosition();
        nameEdit->setText(normalized);
        nameEdit->setCursorPosition(qMin(cursorPosition, normalized.length()));
    });

    phoneEdit = new QLineEdit();
    phoneEdit->setMaxLength(11);
    phoneEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d{0,11}"), phoneEdit));
    phoneEdit->setPlaceholderText("Телефон");

    // Кнопки
    addButton = new QPushButton("Добавить");
    deleteButton = new QPushButton("Удалить");

    // Таблица
    table = new QTableWidget();
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({"Имя", "Телефон"});
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

        QTableWidgetItem *nameItem = new QTableWidgetItem(query.value(1).toString());
        nameItem->setData(Qt::UserRole, query.value(0));

        table->setItem(row, 0, nameItem);
        table->setItem(row, 1, new QTableWidgetItem(query.value(2).toString()));

        row++;
    }
}

// Добавление
void MainWindow::addContact() {
    QString name = nameEdit->text().trimmed();
    QString phone = phoneEdit->text();

    if (name.isEmpty() || phone.isEmpty()) return;

    if (normalizeNameInput(name) != name || countLetters(name) > 10 || countLetters(name) == 0) {
        QMessageBox::warning(this, "Ошибка", "Имя должно состоять только из букв и содержать не более 10 букв");
        return;
    }

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

    int id = table->item(row, 0)->data(Qt::UserRole).toInt();

    QSqlQuery query;
    query.prepare("DELETE FROM contacts WHERE id = :id");
    query.bindValue(":id", id);
    query.exec();

    loadData();
}
