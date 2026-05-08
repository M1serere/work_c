#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QHeaderView>
#include <QMessageBox>
#include <QSignalBlocker>

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

QString phoneDigits(const QString &input) {
    QString digits;

    for (const QChar &ch : input) {
        if (!ch.isDigit()) {
            continue;
        }

        digits.append(ch);
    }

    if (input.startsWith("+7")) {
        digits.remove(0, 1);
    } else if (digits.length() > 10 && (digits[0] == QLatin1Char('7') || digits[0] == QLatin1Char('8'))) {
        digits.remove(0, 1);
    }

    return digits.left(10);
}

QString formatPhoneNumber(const QString &digits) {
    if (digits.isEmpty()) {
        return "";
    }

    QString result = "+7";

    if (digits.length() >= 1) {
        result += " (" + digits.left(qMin(3, digits.length()));
    }

    if (digits.length() >= 3) {
        result += ")";
    }

    if (digits.length() >= 4) {
        result += " " + digits.mid(3, qMin(3, digits.length() - 3));
    }

    if (digits.length() >= 7) {
        result += "-" + digits.mid(6, qMin(2, digits.length() - 6));
    }

    if (digits.length() >= 9) {
        result += "-" + digits.mid(8, qMin(2, digits.length() - 8));
    }

    return result;
}

int digitsBeforePosition(const QString &text, int position) {
    int count = 0;

    for (int i = 0; i < position && i < text.length(); ++i) {
        if (text[i].isDigit()) {
            count++;
        }
    }

    if (text.startsWith("+7") && position >= 2 && count > 0) {
        count--;
    }

    return count;
}

int positionAfterDigits(const QString &text, int digitsCount) {
    if (digitsCount <= 0) {
        return 0;
    }

    int seenDigits = 0;
    bool skippedCountryCode = false;

    for (int i = 0; i < text.length(); ++i) {
        if (text[i].isDigit()) {
            if (text.startsWith("+7") && !skippedCountryCode) {
                skippedCountryCode = true;
                continue;
            }

            seenDigits++;
        }

        if (seenDigits == digitsCount) {
            return i + 1;
        }
    }

    return text.length();
}

void connectPhoneFormatter(QLineEdit *edit) {
    QObject::connect(edit, &QLineEdit::textChanged, edit, [edit](const QString &text) {
        int cursorPosition = edit->cursorPosition();
        int digitsBeforeCursor = digitsBeforePosition(text, cursorPosition);
        QString formatted = formatPhoneNumber(phoneDigits(text));

        if (formatted == text) {
            return;
        }

        QSignalBlocker blocker(edit);
        edit->setText(formatted);
        edit->setCursorPosition(positionAfterDigits(formatted, digitsBeforeCursor));
    });
}

bool isValidNameInput(const QString &input, int maxLetters) {
    return normalizeNameInput(input, maxLetters) == input
        && countLetters(input) > 0
        && countLetters(input) <= maxLetters;
}

int widthForLetters(QTableWidget *table, int lettersCount) {
    return table->fontMetrics().horizontalAdvance(QString(lettersCount, QLatin1Char('W'))) + 40;
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
    phoneEdit->setMaxLength(18);
    phoneEdit->setPlaceholderText("+7 (999) 999-99-99");
    connectPhoneFormatter(phoneEdit);

    // Кнопки
    addButton = new QPushButton("Добавить");
    deleteButton = new QPushButton("Удалить");
    clearButton = new QPushButton("Очистить");

    // Таблица
    table = new QTableWidget();
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"", "Фамилия", "Имя", "Отчество", "Телефон"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);
    table->setStyleSheet(
        "QTableWidget::item:selected {"
        "background-color: #2f80ed;"
        "color: white;"
        "}"
    );
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setTextElideMode(Qt::ElideNone);
    table->setColumnWidth(0, 42);
    table->setColumnWidth(1, widthForLetters(table, 10));
    table->setColumnWidth(2, widthForLetters(table, 15));
    table->setColumnWidth(3, widthForLetters(table, 15));

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

        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setData(Qt::UserRole, query.value(0));
        checkItem->setCheckState(Qt::Unchecked);
        checkItem->setTextAlignment(Qt::AlignCenter);
        checkItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);

        table->setItem(row, 0, checkItem);
        table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        table->setItem(row, 4, new QTableWidgetItem(formatPhoneNumber(phoneDigits(query.value(4).toString()))));

        row++;
    }

    table->setColumnWidth(0, 42);
    table->setColumnWidth(1, widthForLetters(table, 10));
    table->setColumnWidth(2, widthForLetters(table, 15));
    table->setColumnWidth(3, widthForLetters(table, 15));
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

    if (phoneDigits(phone).length() != 10) {
        QMessageBox::warning(this, "Ошибка", "Телефон должен быть в формате +7 (999) 999-99-99");
        return;
    }

    phone = formatPhoneNumber(phoneDigits(phone));

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
    QList<int> checkedIds;

    for (int row = 0; row < table->rowCount(); ++row) {
        QTableWidgetItem *item = table->item(row, 0);
        if (item && item->checkState() == Qt::Checked) {
            checkedIds.append(item->data(Qt::UserRole).toInt());
        }
    }

    if (checkedIds.isEmpty()) return;

    QSqlQuery query;
    query.prepare("DELETE FROM contacts WHERE id = :id");

    for (int id : checkedIds) {
        query.bindValue(":id", id);
        query.exec();
    }

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
