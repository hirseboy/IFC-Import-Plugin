/********************************************************************************
** Form generated from reading UI file 'ImportWizard.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMPORTWIZARD_H
#define UI_IMPORTWIZARD_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWizard>
#include <QtWidgets/QWizardPage>

QT_BEGIN_NAMESPACE

class Ui_ImportWizard
{
public:
    QWizardPage *wizardPage2;

    void setupUi(QWizard *ImportWizard)
    {
        if (ImportWizard->objectName().isEmpty())
            ImportWizard->setObjectName(QString::fromUtf8("ImportWizard"));
        ImportWizard->resize(483, 324);
        wizardPage2 = new QWizardPage();
        wizardPage2->setObjectName(QString::fromUtf8("wizardPage2"));
        ImportWizard->addPage(wizardPage2);

        retranslateUi(ImportWizard);

        QMetaObject::connectSlotsByName(ImportWizard);
    } // setupUi

    void retranslateUi(QWizard *ImportWizard)
    {
        ImportWizard->setWindowTitle(QCoreApplication::translate("ImportWizard", "Wizard", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ImportWizard: public Ui_ImportWizard {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMPORTWIZARD_H
