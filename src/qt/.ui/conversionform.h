/****************************************************************************
** Form interface generated from reading ui file 'conversionform.ui'
**
** Created: 金  7月 18 20:20:48 2003
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.2   edited Dec 19 11:45 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CONVERSIONFORM_H
#define CONVERSIONFORM_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QPushButton;
class QLabel;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QCheckBox;

class ConversionForm : public QDialog
{
    Q_OBJECT

public:
    ConversionForm( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ConversionForm();

    QPushButton* clearPushButton;
    QPushButton* calculatePushButton;
    QPushButton* quitPushButton;
    QLabel* textLabel4;
    QLineEdit* numberLineEdit;
    QLabel* textLabel2;
    QLabel* textLabel5;
    QLabel* textLabel1;
    QComboBox* fromComboBox;
    QLabel* textLabel3;
    QLineEdit* resultLineEdit;
    QSpinBox* decimalsSpinBox;
    QComboBox* toComboBox;
    QCheckBox* checkBox1;

public slots:
    virtual void convert();

protected:
    QHBoxLayout* layout4;
    QHBoxLayout* layout2;

protected slots:
    virtual void languageChange();

private:
    void init();

};

#endif // CONVERSIONFORM_H
