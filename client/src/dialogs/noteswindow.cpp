#include "noteswindow.h"
#include "ui_noteswindow.h"
#include "kdcrmutils.h"
#include "clientsettings.h"

#include "kdcrmdata/sugarnote.h"
#include "kdcrmdata/sugaremail.h"

#include <QScrollBar>

NotesWindow::NotesWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NotesWindow)
{
    ui->setupUi(this);
    ui->textEdit->enableFindReplace(true);
    ui->textEdit->setAcceptRichText(true);
    ClientSettings::self()->restoreWindowSize("NotesWindow", this);
}

NotesWindow::~NotesWindow()
{
    ClientSettings::self()->saveWindowSize("NotesWindow", this);
    delete ui;
}

void NotesWindow::addNote(const SugarNote &note)
{
    const QDateTime modified = KDCRMUtils::dateTimeFromString(note.dateModified());
    QString htmlHeader;
    htmlHeader += (QString("<html><h1>Note by %1, last modified %2:</h1>\n").arg(note.createdByName()).arg(KDCRMUtils::formatDateTime(modified)));
    htmlHeader += "<h2>" + note.name() + "</h2>\n"; // called "Subject" in the web gui
    QString text;
    if (!note.description().isEmpty()) {
        text += note.description() + '\n';
    }
    text += '\n';
    m_notes.append(NoteText(modified, htmlHeader, text));
}

void NotesWindow::addEmail(const SugarEmail &email)
{
    const QString toList = email.toAddrNames();
    const QDateTime dateSent = KDCRMUtils::dateTimeFromString(email.dateSent());
    QString htmlHeader;
    htmlHeader += QString("<html><h1>Mail from %1. Date: %2</h1>\n").arg(email.fromAddrName().trimmed(), KDCRMUtils::formatDateTime(dateSent));
    htmlHeader += QString("<h2>Subject: %1</h2>\n").arg(email.name());
    htmlHeader += QString("<p>To: %1</p>\n").arg(toList);
    const QString text = email.description() + '\n';
    m_notes.append(NoteText(dateSent, htmlHeader, text));
}

void NotesWindow::setVisible(bool visible)
{
    if (ui->textEdit->document()->isEmpty()) {
        qSort(m_notes);
        QTextCursor cursor = ui->textEdit->textCursor();
        foreach (const NoteText &note, m_notes) {
            //cursor.insertText(s_separator);
            cursor.insertHtml("<hr>");
            cursor.insertBlock();
            cursor.setBlockFormat(QTextBlockFormat());
            cursor.insertHtml(note.htmlHeader());
            cursor.setBlockFormat(QTextBlockFormat());
            cursor.insertBlock();
            cursor.insertBlock();
            cursor.insertText(note.text());
        }
    }
    QWidget::setVisible(visible);
    ui->textEdit->verticalScrollBar()->setValue(0);
}

void NotesWindow::on_buttonBox_rejected()
{
    QWidget::close();
}
