#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QtWidgets/QMainWindow>

class MainWindow : public QMainWindow
{
	Q_OBJECT

private:
	void _initMenuBar();
	void _setupLayout();

public:
	MainWindow(QWidget* parent = 0);
	~MainWindow();

	void init();

	void showAbout();
};

#endif //MAINWINDOW_H_
