#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow   w;

  // Set theme
  QFile f(":/theme.qss");

  if (!f.exists()) {
      //qWarning() << "Unable to set stylesheet, file not found";
  } else {
      f.open(QFile::ReadOnly | QFile::Text);
      QTextStream ts(&f);
      a.setStyleSheet(ts.readAll());
  }

  w.show();
  return a.exec();
}
