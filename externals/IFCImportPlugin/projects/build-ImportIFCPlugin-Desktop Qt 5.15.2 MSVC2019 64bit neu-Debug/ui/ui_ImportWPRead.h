/********************************************************************************
** Form generated from reading UI file 'ImportWPRead.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMPORTWPREAD_H
#define UI_IMPORTWPREAD_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWizardPage>

QT_BEGIN_NAMESPACE

class Ui_ImportWPRead
{
public:
    QGridLayout *gridLayout;
    QLineEdit *lineEditIFCFile;
    QToolButton *toolButtonOpenIFCFile;
    QPushButton *pushButtonRead;
    QSpacerItem *horizontalSpacer;
    QTextEdit *textEdit;

    void setupUi(QWizardPage *ImportWPRead)
    {
        if (ImportWPRead->objectName().isEmpty())
            ImportWPRead->setObjectName(QString::fromUtf8("ImportWPRead"));
        ImportWPRead->resize(436, 325);
        gridLayout = new QGridLayout(ImportWPRead);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        lineEditIFCFile = new QLineEdit(ImportWPRead);
        lineEditIFCFile->setObjectName(QString::fromUtf8("lineEditIFCFile"));

        gridLayout->addWidget(lineEditIFCFile, 0, 0, 1, 2);

        toolButtonOpenIFCFile = new QToolButton(ImportWPRead);
        toolButtonOpenIFCFile->setObjectName(QString::fromUtf8("toolButtonOpenIFCFile"));

        gridLayout->addWidget(toolButtonOpenIFCFile, 0, 2, 1, 1);

        pushButtonRead = new QPushButton(ImportWPRead);
        pushButtonRead->setObjectName(QString::fromUtf8("pushButtonRead"));

        gridLayout->addWidget(pushButtonRead, 1, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(329, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 1, 1, 1, 2);

        textEdit = new QTextEdit(ImportWPRead);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));

        gridLayout->addWidget(textEdit, 2, 0, 1, 3);

        gridLayout->setColumnStretch(1, 1);

        retranslateUi(ImportWPRead);

        QMetaObject::connectSlotsByName(ImportWPRead);
    } // setupUi

    void retranslateUi(QWizardPage *ImportWPRead)
    {
        ImportWPRead->setWindowTitle(QCoreApplication::translate("ImportWPRead", "WizardPage", nullptr));
        toolButtonOpenIFCFile->setText(QCoreApplication::translate("ImportWPRead", "...", nullptr));
        pushButtonRead->setText(QCoreApplication::translate("ImportWPRead", "Read", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ImportWPRead: public Ui_ImportWPRead {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMPORTWPREAD_H
