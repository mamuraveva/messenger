#include "connect_dialog.h"
#include "main_window.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName("messengee");
  ConnectDialog dialog;
  if (dialog.exec() != QDialog::Accepted) {
    return 0;
  }
  MainWindow window(
    dialog.username());
  QString err = window.connectToServer(dialog.host(), dialog.port());
  if (!err.isEmpty()) {
    QMessageBox::critical(nullptr, "Ошибка подключения", err);
    return 1;
  }
  window.show();
  return app.exec();
}