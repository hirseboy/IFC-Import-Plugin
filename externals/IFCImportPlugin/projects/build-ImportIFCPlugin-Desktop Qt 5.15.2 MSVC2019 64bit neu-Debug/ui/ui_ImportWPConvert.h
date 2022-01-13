/********************************************************************************
** Form generated from reading UI file 'ImportWPConvert.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMPORTWPCONVERT_H
#define UI_IMPORTWPCONVERT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWizardPage>

QT_BEGIN_NAMESPACE

class Ui_ImportWPConvert
{
public:
    QGridLayout *gridLayout;
    QTextEdit *textEdit;
    QPushButton *pushButtonConvert;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWizardPage *ImportWPConvert)
    {
        if (ImportWPConvert->objectName().isEmpty())
            ImportWPConvert->setObjectName(QString::fromUtf8("ImportWPConvert"));
        ImportWPConvert->resize(436, 325);
        gridLayout = new QGridLayout(ImportWPConvert);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        textEdit = new QTextEdit(ImportWPConvert);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));

        gridLayout->addWidget(textEdit, 1, 0, 1, 3);

        pushButtonConvert = new QPushButton(ImportWPConvert);
        pushButtonConvert->setObjectName(QString::fromUtf8("pushButtonConvert"));

        gridLayout->addWidget(pushButtonConvert, 0, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(329, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 0, 1, 1, 2);


        retranslateUi(ImportWPConvert);

        QMetaObject::connectSlotsByName(ImportWPConvert);
    } // setupUi

    void retranslateUi(QWizardPage *ImportWPConvert)
    {
        ImportWPConvert->setWindowTitle(QCoreApplication::translate("ImportWPConvert", "WizardPage", nullptr));
        pushButtonConvert->setText(QCoreApplication::translate("ImportWPConvert", "Convert", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ImportWPConvert: public Ui_ImportWPConvert {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMPORTWPCONVERT_H
