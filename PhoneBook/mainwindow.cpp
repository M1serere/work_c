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

bool isValidOptionalNameInput(const QString &input, int maxLetters) {
    return input.isEmpty() || isValidNameInput(input, maxLetters);
}

int widthForLetters(QTableWidget *table, int lettersCount) {
    return table->fontMetrics().horizontalAdvance(QString(lettersCount, QLatin1Char('W'))) + 40;
}

bool phoneExists(const QString &normalizedPhone, int excludedId = -1) {
    QSqlQuery query;
    query.prepare("SELECT id, phone FROM contacts");
    query.exec();

    while (query.next()) {
        if (excludedId != -1 && query.value(0).toInt() == excludedId) {
            continue;
        }

        if (phoneDigits(query.value(1).toString()) == normalizedPhone) {
            return true;
        }
    }

    return false;
}
}

// Конструктор окна
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    resize(900, 520);
    setMinimumWidth(820);

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

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Поиск по любому полю");

    // Кнопки
    addButton = new QPushButton("Добавить");
    editButton = new QPushButton("Изменить");
    deleteButton = new QPushButton("Удалить");
    clearButton = new QPushButton("Очистить");
    deleteAllButton = new QPushButton("Удалить всё");

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
    table->setColumnWidth(4, 170);

    // Layout для ввода
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(nameEdit);
    inputLayout->addWidget(firstNameEdit);
    inputLayout->addWidget(patronymicEdit);
    inputLayout->addWidget(phoneEdit);
    inputLayout->addWidget(clearButton);
    inputLayout->addWidget(addButton);
    inputLayout->addWidget(editButton);
    inputLayout->addWidget(deleteButton);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(searchEdit, 1);
    searchLayout->addWidget(deleteAllButton, 1);

    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(table);

    central->setLayout(mainLayout);

    // Связь кнопок
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addContact);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editContact);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteContact);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearInputFields);
    connect(deleteAllButton, &QPushButton::clicked, this, &MainWindow::clearContacts);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::loadData);
    connect(table, &QTableWidget::itemSelectionChanged, this, &MainWindow::fillInputsFromCurrentRow);

    loadData();
}

// Загрузка данных
void MainWindow::loadData() {
    table->setRowCount(0);

    QSqlQuery query;
    const QString searchText = searchEdit->text().trimmed();

    if (searchText.isEmpty()) {
        query.prepare("SELECT id, name, first_name, patronymic, phone FROM contacts");
    } else {
        query.prepare(
            "SELECT id, name, first_name, patronymic, phone FROM contacts "
            "WHERE name LIKE :search "
            "OR first_name LIKE :search "
            "OR patronymic LIKE :search "
            "OR phone LIKE :search "
            "OR (:digits != '' AND replace(replace(replace(replace(replace(phone, '+', ''), ' ', ''), '(', ''), ')', ''), '-', '') LIKE :digits)"
        );
        const QString digits = phoneDigits(searchText);
        query.bindValue(":search", "%" + searchText + "%");
        query.bindValue(":digits", digits.isEmpty() ? QString() : "%" + digits + "%");
    }

    query.exec();

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
    table->setColumnWidth(4, 170);
}

int MainWindow::currentContactId() const {
    if (!table->selectionModel() || table->selectionModel()->selectedRows().isEmpty()) {
        return -1;
    }

    const int row = table->currentRow();
    if (row < 0) {
        return -1;
    }

    QTableWidgetItem *idItem = table->item(row, 0);
    if (!idItem) {
        return -1;
    }

    return idItem->data(Qt::UserRole).toInt();
}

// Добавление
void MainWindow::addContact() {
    QString name = nameEdit->text().trimmed();
    QString firstName = firstNameEdit->text().trimmed();
    QString patronymic = patronymicEdit->text().trimmed();
    QString phone = phoneEdit->text();

    if (firstName.isEmpty() || phone.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Поля имя и телефон обязательны для заполнения");
        return;
    }

    if (!isValidOptionalNameInput(name, 10)) {
        QMessageBox::warning(this, "Ошибка", "Фамилия должна состоять только из букв и содержать не более 10 букв");
        return;
    }

    if (!isValidNameInput(firstName, 15)) {
        QMessageBox::warning(this, "Ошибка", "Имя должно состоять только из букв и содержать не более 15 букв");
        return;
    }

    if (!isValidOptionalNameInput(patronymic, 15)) {
        QMessageBox::warning(this, "Ошибка", "Отчество должно состоять только из букв и содержать не более 15 букв");
        return;
    }

    if (phoneDigits(phone).length() != 10) {
        QMessageBox::warning(this, "Ошибка", "Телефон должен быть в формате +7 (999) 999-99-99");
        return;
    }

    const QString normalizedPhone = phoneDigits(phone);
    if (phoneExists(normalizedPhone)) {
        QMessageBox::warning(this, "Ошибка", "Запись с таким номером телефона уже существует");
        return;
    }

    phone = formatPhoneNumber(normalizedPhone);

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

void MainWindow::editContact() {
    const int id = currentContactId();
    if (id == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите запись для изменения");
        return;
    }

    QString name = nameEdit->text().trimmed();
    QString firstName = firstNameEdit->text().trimmed();
    QString patronymic = patronymicEdit->text().trimmed();
    QString phone = phoneEdit->text();

    if (firstName.isEmpty() || phone.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Поля имя и телефон обязательны для заполнения");
        return;
    }

    if (!isValidOptionalNameInput(name, 10)) {
        QMessageBox::warning(this, "Ошибка", "Фамилия должна состоять только из букв и содержать не более 10 букв");
        return;
    }

    if (!isValidNameInput(firstName, 15)) {
        QMessageBox::warning(this, "Ошибка", "Имя должно состоять только из букв и содержать не более 15 букв");
        return;
    }

    if (!isValidOptionalNameInput(patronymic, 15)) {
        QMessageBox::warning(this, "Ошибка", "Отчество должно состоять только из букв и содержать не более 15 букв");
        return;
    }

    if (phoneDigits(phone).length() != 10) {
        QMessageBox::warning(this, "Ошибка", "Телефон должен быть в формате +7 (999) 999-99-99");
        return;
    }

    const QString normalizedPhone = phoneDigits(phone);
    if (phoneExists(normalizedPhone, id)) {
        QMessageBox::warning(this, "Ошибка", "Запись с таким номером телефона уже существует");
        return;
    }

    phone = formatPhoneNumber(normalizedPhone);

    QSqlQuery query;
    query.prepare("UPDATE contacts SET name = :name, first_name = :first_name, "
                  "patronymic = :patronymic, phone = :phone WHERE id = :id");
    query.bindValue(":name", name);
    query.bindValue(":first_name", firstName);
    query.bindValue(":patronymic", patronymic);
    query.bindValue(":phone", phone);
    query.bindValue(":id", id);
    query.exec();

    loadData();
}

void MainWindow::fillInputsFromCurrentRow() {
    if (!table->selectionModel() || table->selectionModel()->selectedRows().isEmpty()) {
        return;
    }

    const int row = table->currentRow();
    if (row < 0) {
        return;
    }

    nameEdit->setText(table->item(row, 1) ? table->item(row, 1)->text() : QString());
    firstNameEdit->setText(table->item(row, 2) ? table->item(row, 2)->text() : QString());
    patronymicEdit->setText(table->item(row, 3) ? table->item(row, 3)->text() : QString());
    phoneEdit->setText(table->item(row, 4) ? table->item(row, 4)->text() : QString());
}

void MainWindow::clearInputFields() {
    table->clearSelection();
    nameEdit->clear();
    firstNameEdit->clear();
    patronymicEdit->clear();
    phoneEdit->clear();
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

    clearInputFields();
    loadData();
}
