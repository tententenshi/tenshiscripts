#include <qapplication.h>
#include "conversionform.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    ConversionForm w;
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
