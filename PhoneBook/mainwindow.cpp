#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QHeaderView>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

namespace {
QString normalizeNameInput(const QString &input, int maxLetters) {
    QString result;
    int lettersCount = 0;

    for (const QChar &ch : input) {
        if (ch == QLatin1Char(' ')) {
            result.append(ch);
            continue;
        }

        if (!ch.isLetter() || lettersCount >= maxLetters) {
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

void connectNameLimiter(QLineEdit *edit, int maxLetters) {
    QObject::connect(edit, &QLineEdit::textChanged, edit, [edit, maxLetters](const QString &text) {
        QString normalized = normalizeNameInput(text, maxLetters);
        if (normalized == text) {
            return;
        }

        int cursorPosition = edit->cursorPosition();
        edit->setText(normalized);
        edit->setCursorPosition(qMin(cursorPosition, normalized.length()));
    });
}

bool isValidNameInput(const QString &input, int maxLetters) {
    return normalizeNameInput(input, maxLetters) == input
        && countLetters(input) > 0
        && countLetters(input) <= maxLetters;
}
}

// Конструктор окна
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

    QWidget *central = new QWidget();
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout();

    // Поля ввода
    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("Фамилия");
    connectNameLimiter(nameEdit, 10);

    firstNameEdit = new QLineEdit();
    firstNameEdit->setPlaceholderText("Имя");
    connectNameLimiter(firstNameEdit, 15);

    patronymicEdit = new QLineEdit();
    patronymicEdit->setPlaceholderText("Отчество");
    connectNameLimiter(patronymicEdit, 15);

    phoneEdit = new QLineEdit();
    phoneEdit->setMaxLength(11);
    phoneEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d{0,11}"), phoneEdit));
    phoneEdit->setPlaceholderText("Телефон");

    // Кнопки
    addButton = new QPushButton("Добавить");
    deleteButton = new QPushButton("Удалить");
    clearButton = new QPushButton("Очистить");

    // Таблица
    table = new QTableWidget();
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"Фамилия", "Имя", "Отчество", "Телефон"});
    table->horizontalHeader()->setStretchLastSection(true);

    // Layout для ввода
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(nameEdit);
    inputLayout->addWidget(firstNameEdit);
    inputLayout->addWidget(patronymicEdit);
    inputLayout->addWidget(phoneEdit);
    inputLayout->addWidget(addButton);
    inputLayout->addWidget(deleteButton);
    inputLayout->addWidget(clearButton);

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(table);

    central->setLayout(mainLayout);

    // Связь кнопок
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addContact);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteContact);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearContacts);

    loadData();
}

// Загрузка данных
void MainWindow::loadData() {
    table->setRowCount(0);

    QSqlQuery query("SELECT id, name, first_name, patronymic, phone FROM contacts");

    int row = 0;
    while (query.next()) {
        table->insertRow(row);

        QTableWidgetItem *nameItem = new QTableWidgetItem(query.value(1).toString());
        nameItem->setData(Qt::UserRole, query.value(0));

        table->setItem(row, 0, nameItem);
        table->setItem(row, 1, new QTableWidgetItem(query.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(query.value(3).toString()));
        table->setItem(row, 3, new QTableWidgetItem(query.value(4).toString()));

        row++;
    }
}

// Добавление
void MainWindow::addContact() {
    QString name = nameEdit->text().trimmed();
    QString firstName = firstNameEdit->text().trimmed();
    QString patronymic = patronymicEdit->text().trimmed();
    QString phone = phoneEdit->text();

    if (name.isEmpty() || firstName.isEmpty() || patronymic.isEmpty() || phone.isEmpty()) return;

    if (!isValidNameInput(name, 10)) {
        QMessageBox::warning(this, "Ошибка", "Фамилия должна состоять только из букв и содержать не более 10 букв");
        return;
    }

    if (!isValidNameInput(firstName, 15)) {
        QMessageBox::warning(this, "Ошибка", "Имя должно состоять только из букв и содержать не более 15 букв");
        return;
    }

    if (!isValidNameInput(patronymic, 15)) {
        QMessageBox::warning(this, "Ошибка", "Отчество должно состоять только из букв и содержать не более 15 букв");
        return;
    }

    if (!QRegularExpression("^\\d{11}$").match(phone).hasMatch()) {
        QMessageBox::warning(this, "Ошибка", "Телефон должен содержать ровно 11 цифр");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO contacts (name, first_name, patronymic, phone) "
                  "VALUES (:name, :first_name, :patronymic, :phone)");
    query.bindValue(":name", name);
    query.bindValue(":first_name", firstName);
    query.bindValue(":patronymic", patronymic);
    query.bindValue(":phone", phone);
    query.exec();

    nameEdit->clear();
    firstNameEdit->clear();
    patronymicEdit->clear();
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

// Очистка телефонной книги
void MainWindow::clearContacts() {
    if (table->rowCount() == 0) return;

    QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "Подтверждение",
        "Очистить всю телефонную книгу?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (answer != QMessageBox::Yes) {
        return;
    }

    QSqlQuery query;
    query.exec("DELETE FROM contacts");

    loadData();
}
