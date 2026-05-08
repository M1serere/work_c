#include "mainwindow.h"

#include <QAction>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QSqlQuery>
#include <QStyle>
#include <QVBoxLayout>

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
        if (ch.isDigit()) {
            digits.append(ch);
        }
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

QLabel *fieldLabel(const QString &text) {
    QLabel *label = new QLabel(text);
    label->setObjectName("fieldLabel");
    return label;
}

QFrame *sectionFrame() {
    QFrame *frame = new QFrame();
    frame->setObjectName("sectionFrame");
    return frame;
}
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    resize(1240, 760);
    setMinimumSize(980, 620);
    setWindowTitle("PhoneBook");

    QWidget *central = new QWidget();
    central->setObjectName("central");
    setCentralWidget(central);

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
    searchEdit->setObjectName("searchEdit");
    searchEdit->setPlaceholderText("Поиск по фамилии, имени, телефону...");
    searchEdit->addAction(style()->standardIcon(QStyle::SP_FileDialogContentsView), QLineEdit::LeadingPosition);

    addButton = new QPushButton("Добавить");
    addButton->setObjectName("primaryButton");
    addButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));

    editButton = new QPushButton("Сохранить изменения");
    editButton->setObjectName("outlineButton");
    editButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));

    clearButton = new QPushButton("Очистить поля");
    clearButton->setObjectName("plainButton");
    clearButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));

    deleteButton = new QPushButton("Удалить выбранный");
    deleteButton->setObjectName("dangerButton");
    deleteButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));

    deleteAllButton = new QPushButton("Удалить все контакты");
    deleteAllButton->setObjectName("dangerOutlineButton");
    deleteAllButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));

    table = new QTableWidget();
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"Фамилия", "Имя", "Отчество", "Телефон"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(false);
    table->setShowGrid(true);
    table->setTextElideMode(Qt::ElideNone);
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(48);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setMinimumHeight(46);

    emptyHintLabel = new QLabel("▣\nКонтактов пока немного");
    emptyHintLabel->setObjectName("emptyHintLabel");
    emptyHintLabel->setAlignment(Qt::AlignCenter);

    countLabel = new QLabel("Всего контактов: 0");
    countLabel->setObjectName("countLabel");

    QFrame *editorFrame = sectionFrame();
    QVBoxLayout *editorLayout = new QVBoxLayout(editorFrame);
    editorLayout->setContentsMargins(20, 16, 20, 20);
    editorLayout->setSpacing(18);

    QLabel *editorTitle = new QLabel("Телефонная книга");
    editorTitle->setObjectName("sectionTitle");
    editorLayout->addWidget(editorTitle);

    QGridLayout *fieldsLayout = new QGridLayout();
    fieldsLayout->setHorizontalSpacing(16);
    fieldsLayout->setVerticalSpacing(8);
    fieldsLayout->addWidget(fieldLabel("Фамилия"), 0, 0);
    fieldsLayout->addWidget(fieldLabel("Имя"), 0, 1);
    fieldsLayout->addWidget(fieldLabel("Отчество"), 0, 2);
    fieldsLayout->addWidget(fieldLabel("Телефон"), 0, 3);
    fieldsLayout->addWidget(nameEdit, 1, 0);
    fieldsLayout->addWidget(firstNameEdit, 1, 1);
    fieldsLayout->addWidget(patronymicEdit, 1, 2);
    fieldsLayout->addWidget(phoneEdit, 1, 3);
    editorLayout->addLayout(fieldsLayout);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(14);
    buttonsLayout->addWidget(addButton);
    buttonsLayout->addWidget(editButton);
    buttonsLayout->addWidget(clearButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(deleteButton);
    editorLayout->addLayout(buttonsLayout);

    QFrame *listFrame = sectionFrame();
    QVBoxLayout *listLayout = new QVBoxLayout(listFrame);
    listLayout->setContentsMargins(20, 18, 20, 14);
    listLayout->setSpacing(14);

    QLabel *listTitle = new QLabel("Список контактов");
    listTitle->setObjectName("sectionTitle");
    listLayout->addWidget(listTitle);
    listLayout->addWidget(searchEdit, 0, Qt::AlignLeft);
    listLayout->addWidget(table, 1);
    listLayout->addWidget(emptyHintLabel, 0);

    QHBoxLayout *footerLayout = new QHBoxLayout();
    footerLayout->addWidget(countLabel);
    footerLayout->addStretch();
    footerLayout->addWidget(deleteAllButton);
    listLayout->addLayout(footerLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(16);
    mainLayout->addWidget(editorFrame);
    mainLayout->addWidget(listFrame, 1);

    central->setStyleSheet(R"(
        QWidget#central {
            background: #f7f8fb;
            color: #111827;
            font-family: "Segoe UI";
            font-size: 16px;
        }

        QFrame#sectionFrame {
            background: #ffffff;
            border: 1px solid #d8dde6;
            border-radius: 7px;
        }

        QLabel#sectionTitle {
            color: #0f58b7;
            font-size: 18px;
            font-weight: 700;
        }

        QLabel#fieldLabel {
            color: #111827;
            font-size: 16px;
        }

        QLineEdit {
            min-height: 40px;
            padding: 5px 12px;
            border: 1px solid #cfd5de;
            border-radius: 5px;
            background: #ffffff;
            selection-background-color: #dbeafe;
        }

        QLineEdit:focus {
            border: 1px solid #2a7de1;
        }

        QLineEdit#searchEdit {
            min-width: 470px;
            padding-left: 8px;
            color: #111827;
        }

        QPushButton {
            min-height: 40px;
            padding: 0 16px;
            border-radius: 5px;
            font-weight: 600;
        }

        QPushButton#primaryButton {
            color: white;
            border: 1px solid #1769c8;
            background: #1976d2;
        }

        QPushButton#primaryButton:hover {
            background: #1569bd;
        }

        QPushButton#outlineButton {
            color: #155eb5;
            border: 1px solid #69a8ee;
            background: #ffffff;
        }

        QPushButton#plainButton {
            color: #20242c;
            border: 1px solid #bec5cf;
            background: #ffffff;
        }

        QPushButton#dangerButton {
            color: white;
            border: 1px solid #ef4444;
            background: #ef4444;
        }

        QPushButton#dangerButton:hover {
            background: #dc2626;
        }

        QPushButton#dangerOutlineButton {
            color: #dc2626;
            border: 1px solid #ef4444;
            background: #ffffff;
        }

        QTableWidget {
            border: 1px solid #d4d9e1;
            border-radius: 5px;
            background: #ffffff;
            gridline-color: #d4d9e1;
            font-size: 16px;
        }

        QHeaderView::section {
            background: #ffffff;
            color: #111827;
            border: 0;
            border-right: 1px solid #d4d9e1;
            border-bottom: 1px solid #d4d9e1;
            padding: 10px 12px;
            font-weight: 700;
            font-size: 16px;
        }

        QTableWidget::item {
            padding: 8px 16px;
            border-bottom: 1px solid #e4e7ec;
        }

        QTableWidget::item:selected {
            background: #e8f2ff;
            color: #111827;
        }

        QLabel#emptyHintLabel {
            color: #8f98a6;
            font-size: 16px;
            min-height: 82px;
        }

        QLabel#countLabel {
            color: #111827;
            font-size: 16px;
        }
    )");

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addContact);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editContact);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteContact);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearInputFields);
    connect(deleteAllButton, &QPushButton::clicked, this, &MainWindow::clearContacts);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::loadData);
    connect(table, &QTableWidget::itemSelectionChanged, this, &MainWindow::fillInputsFromCurrentRow);

    loadData();
}

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

        QTableWidgetItem *nameItem = new QTableWidgetItem(query.value(1).toString());
        nameItem->setData(Qt::UserRole, query.value(0));

        table->setItem(row, 0, nameItem);
        table->setItem(row, 1, new QTableWidgetItem(query.value(2).toString()));
        table->setItem(row, 2, new QTableWidgetItem(query.value(3).toString()));
        table->setItem(row, 3, new QTableWidgetItem(formatPhoneNumber(phoneDigits(query.value(4).toString()))));

        row++;
    }

    updateContactSummary();
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

void MainWindow::updateContactSummary() {
    QSqlQuery countQuery("SELECT COUNT(*) FROM contacts");
    int total = 0;
    if (countQuery.next()) {
        total = countQuery.value(0).toInt();
    }

    countLabel->setText(QString("Всего контактов: <span style='color:#0f58b7; font-weight:700;'>%1</span>").arg(total));
    emptyHintLabel->setVisible(table->rowCount() <= 2);
}

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

    clearInputFields();
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

    nameEdit->setText(table->item(row, 0) ? table->item(row, 0)->text() : QString());
    firstNameEdit->setText(table->item(row, 1) ? table->item(row, 1)->text() : QString());
    patronymicEdit->setText(table->item(row, 2) ? table->item(row, 2)->text() : QString());
    phoneEdit->setText(table->item(row, 3) ? table->item(row, 3)->text() : QString());
}

void MainWindow::clearInputFields() {
    table->clearSelection();
    nameEdit->clear();
    firstNameEdit->clear();
    patronymicEdit->clear();
    phoneEdit->clear();
}

void MainWindow::deleteContact() {
    const int id = currentContactId();
    if (id == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите запись для удаления");
        return;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM contacts WHERE id = :id");
    query.bindValue(":id", id);
    query.exec();

    clearInputFields();
    loadData();
}

void MainWindow::clearContacts() {
    if (table->rowCount() == 0) {
        return;
    }

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
