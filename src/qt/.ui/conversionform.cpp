/****************************************************************************
** Form implementation generated from reading ui file 'conversionform.ui'
**
** Created: 金  7月 18 20:20:53 2003
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.2   edited Dec 19 11:45 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "conversionform.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

#include "../conversionform.ui.h"
/* 
 *  Constructs a ConversionForm as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
ConversionForm::ConversionForm( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "ConversionForm" );

    QWidget* privateLayoutWidget = new QWidget( this, "layout4" );
    privateLayoutWidget->setGeometry( QRect( 11, 314, 438, 30 ) );
    layout4 = new QHBoxLayout( privateLayoutWidget, 11, 6, "layout4"); 

    clearPushButton = new QPushButton( privateLayoutWidget, "clearPushButton" );
    layout4->addWidget( clearPushButton );

    calculatePushButton = new QPushButton( privateLayoutWidget, "calculatePushButton" );
    calculatePushButton->setDefault( TRUE );
    layout4->addWidget( calculatePushButton );
    QSpacerItem* spacer = new QSpacerItem( 21, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout4->addItem( spacer );

    quitPushButton = new QPushButton( privateLayoutWidget, "quitPushButton" );
    layout4->addWidget( quitPushButton );

    textLabel4 = new QLabel( this, "textLabel4" );
    textLabel4->setGeometry( QRect( 12, 102, 111, 24 ) );

    numberLineEdit = new QLineEdit( this, "numberLineEdit" );
    numberLineEdit->setGeometry( QRect( 129, 12, 319, 24 ) );
    numberLineEdit->setAlignment( int( QLineEdit::AlignRight ) );

    textLabel2 = new QLabel( this, "textLabel2" );
    textLabel2->setGeometry( QRect( 12, 42, 111, 24 ) );

    textLabel5 = new QLabel( this, "textLabel5" );
    textLabel5->setGeometry( QRect( 12, 132, 111, 26 ) );

    textLabel1 = new QLabel( this, "textLabel1" );
    textLabel1->setGeometry( QRect( 12, 12, 111, 24 ) );

    fromComboBox = new QComboBox( FALSE, this, "fromComboBox" );
    fromComboBox->setGeometry( QRect( 129, 42, 319, 24 ) );

    textLabel3 = new QLabel( this, "textLabel3" );
    textLabel3->setGeometry( QRect( 12, 72, 111, 24 ) );

    resultLineEdit = new QLineEdit( this, "resultLineEdit" );
    resultLineEdit->setGeometry( QRect( 129, 102, 319, 24 ) );
    resultLineEdit->setPaletteBackgroundColor( QColor( 251, 255, 211 ) );
    resultLineEdit->setAlignment( int( QLineEdit::AlignRight ) );

    QWidget* privateLayoutWidget_2 = new QWidget( this, "layout2" );
    privateLayoutWidget_2->setGeometry( QRect( 129, 132, 319, 26 ) );
    layout2 = new QHBoxLayout( privateLayoutWidget_2, 11, 6, "layout2"); 
    QSpacerItem* spacer_2 = new QSpacerItem( 133, 24, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout2->addItem( spacer_2 );

    decimalsSpinBox = new QSpinBox( privateLayoutWidget_2, "decimalsSpinBox" );
    decimalsSpinBox->setMaxValue( 6 );
    decimalsSpinBox->setValue( 3 );
    layout2->addWidget( decimalsSpinBox );

    toComboBox = new QComboBox( FALSE, this, "toComboBox" );
    toComboBox->setGeometry( QRect( 129, 72, 319, 24 ) );

    checkBox1 = new QCheckBox( this, "checkBox1" );
    checkBox1->setGeometry( QRect( 130, 170, 102, 21 ) );
    languageChange();
    resize( QSize(460, 355).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( clearPushButton, SIGNAL( clicked() ), numberLineEdit, SLOT( clear() ) );
    connect( clearPushButton, SIGNAL( clicked() ), resultLineEdit, SLOT( clear() ) );
    connect( clearPushButton, SIGNAL( clicked() ), numberLineEdit, SLOT( setFocus() ) );
    connect( quitPushButton, SIGNAL( clicked() ), this, SLOT( close() ) );
    connect( calculatePushButton, SIGNAL( clicked() ), this, SLOT( convert() ) );
    connect( decimalsSpinBox, SIGNAL( valueChanged(int) ), this, SLOT( convert() ) );
    connect( fromComboBox, SIGNAL( activated(int) ), this, SLOT( convert() ) );
    connect( toComboBox, SIGNAL( activated(int) ), this, SLOT( convert() ) );
    connect( checkBox1, SIGNAL( toggled(bool) ), resultLineEdit, SLOT( deselect() ) );

    // tab order
    setTabOrder( numberLineEdit, fromComboBox );
    setTabOrder( fromComboBox, toComboBox );
    setTabOrder( toComboBox, resultLineEdit );
    setTabOrder( resultLineEdit, decimalsSpinBox );
    setTabOrder( decimalsSpinBox, clearPushButton );
    setTabOrder( clearPushButton, calculatePushButton );
    setTabOrder( calculatePushButton, quitPushButton );

    // buddies
    textLabel2->setBuddy( fromComboBox );
    textLabel5->setBuddy( decimalsSpinBox );
    textLabel1->setBuddy( numberLineEdit );
    textLabel3->setBuddy( toComboBox );
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
ConversionForm::~ConversionForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ConversionForm::languageChange()
{
    setCaption( tr( "Metric Conversion" ) );
    clearPushButton->setText( tr( "&Clear" ) );
    calculatePushButton->setText( tr( "Calculate" ) );
    quitPushButton->setText( tr( "&Quit" ) );
    textLabel4->setText( tr( "Result:" ) );
    textLabel2->setText( tr( "Convert &From:" ) );
    textLabel5->setText( tr( "&Decimals:" ) );
    textLabel1->setText( tr( "Enter &Number:" ) );
    fromComboBox->clear();
    fromComboBox->insertItem( tr( "Kilometers" ) );
    fromComboBox->insertItem( tr( "Meters" ) );
    fromComboBox->insertItem( tr( "Centimeters" ) );
    fromComboBox->insertItem( tr( "Millimeters" ) );
    textLabel3->setText( tr( "Convert &To:" ) );
    toComboBox->clear();
    toComboBox->insertItem( tr( "Miles" ) );
    toComboBox->insertItem( tr( "Yards" ) );
    toComboBox->insertItem( tr( "Feet" ) );
    toComboBox->insertItem( tr( "Inches" ) );
    checkBox1->setText( tr( "checkBox1" ) );
}

